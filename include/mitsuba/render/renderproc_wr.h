#if !defined(__IMAGEPROC_WR_H)
#define __IMAGEPROC_WR_H

#include <mitsuba/core/sched.h>
#include <mitsuba/render/rfilter.h>

MTS_NAMESPACE_BEGIN

/**
 * This class is used by image-based parallel processes and encapsulates
 * the resulting information in a rectangular region of an image. Such
 * blocks may also include a border storing contributions that are slightly
 * outside -- this is required to support image reconstruction filters. When 
 * used in the context of adaptive sampling, it is possible to limit the 
 * influence of a group of samples that all lie within the same pixel. 
 * This is important to avoid bias when using large-extent reconstruction 
 * filters, while at the same time placing significantly different amounts
 * of samples into nearby pixels.
 */
class MTS_EXPORT_RENDER ImageBlock : public WorkResult {
public:
	/**
	 * Construct a new image block of the requested size
	 *
	 * @param maxBlockSize
	 *    Upper bound on the horizontal & vertical size of a block
	 *
	 * @param borderSize
	 *    Size of the border region storing contributions that
	 *    affect neighboring blocks due to the use of image 
	 *    reconstruction filters
	 *
	 * @param supportWeights
	 *    Should per-pixel weights be supported? (required for
	 *    image reconstruction filters)
	 *
	 * @param supportAlpha
	 *    Should per-pixel alpha values be supported?
	 *
	 * @param supportSnapshot
	 *    Should the snapshot feature be supported?
	 *
	 * @param supportStatistics
	 *    Should per-pixel variance estimates be supported?
	 */
	ImageBlock(const Vector2i &maxBlockSize, int borderSize, 
		bool supportWeights, bool supportAlpha,
		bool supportSnapshot, bool supportStatistics);

	/// Clear everything to zero
	void clear();

	/// Add another image block to this one
	void add(const ImageBlock *block);

	/* WorkResult interface */
	void load(Stream *stream);
	void save(Stream *stream) const;
	std::string toString() const;

	/**
	 * Add a sample to the image block -- returns false if the 
	 * sample contains invalid values (negative/NaN)
	 */
	inline bool putSample(Point2 sample, const Spectrum &spec,
		const Float alphaValue, const TabulatedFilter *filter) {
		/* Implementation based on PBRT */
		const Vector2 filterSize = filter->getFilterSize();

		/* Check for problems with the sample */
		if (!spec.isValid()) {
			Log(EWarn, "Invalid sample value : %s", spec.toString().c_str());
			return false;
		}

		/* Find the affected pixel region in discrete coordinates */
		sample.x = sample.x - 0.5f - (offset.x - border);
		sample.y = sample.y - 0.5f - (offset.y - border);
		int xStart = (int) std::ceil(sample.x - filterSize.x);
		int xEnd   = (int) std::floor(sample.x + filterSize.x);
		int yStart = (int) std::ceil(sample.y - filterSize.y);
		int yEnd   = (int) std::floor(sample.y + filterSize.y);

		xStart = std::max(0, xStart); yStart = std::max(0, yStart);
		xEnd = std::min(xEnd, fullSize.x-1); yEnd = std::min(yEnd, fullSize.y-1);
		const int xWidth = xEnd-xStart+1, yWidth = yEnd-yStart+1;

		/* Precompute array lookup indices and store them the stack (alloca) */
		int *idxX = (int *) alloca(xWidth * sizeof(int));
		int *idxY = (int *) alloca(yWidth * sizeof(int));

		for (int x=xStart; x<=xEnd; ++x) {
			const Float trafoX = filter->getSizeFactor().x * std::abs(x - sample.x);
			idxX[x-xStart] = std::min((int) trafoX, FILTER_RESOLUTION);
		}

		for (int y=yStart; y<=yEnd; ++y) {
			const Float trafoY = filter->getSizeFactor().y * std::abs(y - sample.y);
			idxY[y-yStart] = std::min((int) trafoY, FILTER_RESOLUTION);
		}

		/* Update the weights+pixels */
		for (int y=yStart; y<=yEnd; ++y) {
			int index = y*fullSize.x + xStart;
			for (int x=xStart; x<=xEnd; ++x) {
				Float weight = filter->lookup(idxX[x-xStart], idxY[y-yStart]);
				pixels[index] += spec * weight;
				alpha[index] += alphaValue * weight;
				weights[index++] += weight;
			}
		}
		return true;
	}
	
	/**
	 * Like putSample(), but does not update weights or alpha values
	 */
	inline bool splat(Point2 sample, const Spectrum &spec,
		const TabulatedFilter *filter) {
		/* Implementation based on PBRT */
		const Vector2 filterSize = filter->getFilterSize();

		/* Check for problems with the sample */
		if (!spec.isValid()) {
			Log(EWarn, "Invalid sample value : %s", spec.toString().c_str());
			return false;
		}

		/* Find the affected pixel region in discrete coordinates */
		sample.x = sample.x - 0.5f - (offset.x - border);
		sample.y = sample.y - 0.5f - (offset.y - border);
		int xStart = (int) std::ceil(sample.x - filterSize.x);
		int xEnd   = (int) std::floor(sample.x + filterSize.x);
		int yStart = (int) std::ceil(sample.y - filterSize.y);
		int yEnd   = (int) std::floor(sample.y + filterSize.y);

		xStart = std::max(0, xStart); yStart = std::max(0, yStart);
		xEnd = std::min(xEnd, fullSize.x-1); yEnd = std::min(yEnd, fullSize.y-1);
		const int xWidth = xEnd-xStart+1, yWidth = yEnd-yStart+1;

		/* Precompute array lookup indices and store them the stack (alloca) */
		int *idxX = (int *) alloca(xWidth * sizeof(int));
		int *idxY = (int *) alloca(yWidth * sizeof(int));

		for (int x=xStart; x<=xEnd; ++x) {
			const Float trafoX = filter->getSizeFactor().x * std::abs(x - sample.x);
			idxX[x-xStart] = std::min((int) trafoX, FILTER_RESOLUTION);
		}

		for (int y=yStart; y<=yEnd; ++y) {
			const Float trafoY = filter->getSizeFactor().y * std::abs(y - sample.y);
			idxY[y-yStart] = std::min((int) trafoY, FILTER_RESOLUTION);
		}

		/* Update the weights+pixels */
		for (int y=yStart; y<=yEnd; ++y) {
			int index = y*fullSize.x + xStart;
			for (int x=xStart; x<=xEnd; ++x) {
				Float weight = filter->lookup(idxX[x-xStart], idxY[y-yStart]);
				pixels[index++] += spec * weight;
			}
		}
		return true;
	}

	/**
	 * Before starting to place samples within the area of a single pixel, this
	 * method can be called to take a snapshot of all surrounding spectrum+weight 
	 * values. Those values can later be used to ensure that adjacent pixels will
	 * not be disproportionately biased by this pixel's contributions.
	 */
	inline void snapshot(int px, int py) {
		const int xStart = px - offset.x, xEnd = xStart + 2*border;
		const int yStart = py - offset.y, yEnd = yStart + 2*border;

		int snapshotIndex = 0, pixelIndex;
		for (int y=yStart; y<=yEnd; ++y) {
			pixelIndex = y*fullSize.x + xStart;
			for (int x=xStart; x<=xEnd; ++x) {
				pixelSnapshot[snapshotIndex] = pixels[pixelIndex];
				alphaSnapshot[snapshotIndex] = alpha[pixelIndex];
				weightSnapshot[snapshotIndex] = weights[pixelIndex];
				snapshotIndex++;
				pixelIndex++;
			}
		}
	}
	
	/**
	 * Multiplies the contributions since the last snapshot by the
	 * supplied value.
	 */
	inline void normalize(int px, int py, Float factor) {
		const int xStart = px - offset.x, xEnd = xStart + 2*border;
		const int yStart = py - offset.y, yEnd = yStart + 2*border;
		int snapshotIndex = 0, pixelIndex;

		for (int y=yStart; y<=yEnd; ++y) {
			pixelIndex = y*fullSize.x + xStart;
			for (int x=xStart; x<=xEnd; ++x) {
				pixels[pixelIndex] = pixelSnapshot[snapshotIndex] + 
					(pixels[pixelIndex] - pixelSnapshot[snapshotIndex]) * factor;
				weights[pixelIndex] = weightSnapshot[snapshotIndex] + 
					(weights[pixelIndex] - weightSnapshot[snapshotIndex]) * factor;
				alpha[pixelIndex] = alphaSnapshot[snapshotIndex] + 
					(alpha[pixelIndex] - alphaSnapshot[snapshotIndex]) * factor;
				snapshotIndex++;
				pixelIndex++;
			}
		}
	}

	/// Return whether image variances are also collected
	inline bool collectStatistics() const {
		return variances != NULL;
	}

	/// Test case mode - set the variance of a pixel
	inline void setVariance(int px, int py, Spectrum value, uint32_t sampleCount) {
		Assert(border == 0);
		const int x = px - offset.x, y = py - offset.y;
		variances[y*fullSize.x + x] = value;
		nSamples[y*fullSize.x + x] = sampleCount;
	}

	/// Return the border size
	inline int getBorder() const { return border; }

	/// Return the offset of this image block
	inline const Point2i &getOffset() const { return offset; }

	/// Set the offset of this image block
	inline void setOffset(const Point2i &_offset) {
		offset = _offset;
	}

	/// Return the size of this image block
	inline const Vector2i &getSize() const { return size; }
	
	/// Set the size of this image block
	inline void setSize(const Vector2i &_size) {
		size = _size;
		fullSize.x = size.x + 2*border;
		fullSize.y = size.y + 2*border;
	}

	/// Return the size of this image block (including borders)
	inline const Vector2i &getFullSize() const { return fullSize; }

	/// Look up a pixel (given a 1D array index for performance reasons)
	inline const Spectrum &getPixel(size_t idx) const { return pixels[idx]; }
	
	/// Set the value of a pixel (given a 1D array index for performance reasons)
	inline void setPixel(size_t idx, const Spectrum &spec) { pixels[idx] = spec; }

	/// Look up a weight (given a 1D array index for performance reasons)
	inline Float getWeight(size_t idx) const { return weights[idx]; }
	
	/// Set a weight (given a 1D array index for performance reasons)
	inline void setWeight(size_t idx, Float weight) { weights[idx] = weight; }

	/// Look up a variance value (given a 1D array index for performance reasons)
	inline Spectrum getVariance(size_t idx) const { return variances[idx]; }
	
	/// Look up a sample count (given a 1D array index for performance reasons)
	inline uint32_t getSampleCount(size_t idx) const { return nSamples[idx]; }

	/// Look up an alpha value (given a 1D array index for performance reasons)
	inline Float getAlpha(size_t idx) const { return alpha[idx]; }

	/**
	 * Return the value of the 'extra' field (to be used for storing
	 * special flags etc. associated with this block)
	 */
	inline int32_t getExtra() const { return extra; }

	/**
	 * Set the value of the 'extra' field (to be used for storing
	 * special flags etc. associated with this block)
	 */
	inline void setExtra(int32_t value) { extra = value; }

	MTS_DECLARE_CLASS()
protected:
	/// Virtual destructor
	virtual ~ImageBlock();
protected:
	/* Offset of this image block on the film plane */
	Point2i offset;
	/* Size of this image block (excluding the border) */
	Vector2i size;
	/* Size of this image block (including the border) */
	Vector2i fullSize;
	/* Size of the border */
	int border;
	Vector2i maxBlockSize;
	/* Pixel buffer */
	Spectrum *pixels;
	/* Alpha values */
	Float *alpha;
	/* Pixel weights */
	Float *weights;
	/* Pixel variances */
	Spectrum *variances;
	/* Pixel sample count */
	uint32_t *nSamples;
	/* Pixel snapshot for normalization */
	Spectrum *pixelSnapshot;
	/* Weight snapshot for normalization */
	Float *weightSnapshot;
	/* Alpha snapshot for normalization */
	Float *alphaSnapshot;
	/* Implementation specific payload */
	int32_t extra;
};

MTS_NAMESPACE_END

#endif /* __IMAGEPROC_WR_H */