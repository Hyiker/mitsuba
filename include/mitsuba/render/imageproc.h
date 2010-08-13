#if !defined(__IMAGEPROC_H)
#define __IMAGEPROC_H

#include <mitsuba/core/sched.h>

MTS_NAMESPACE_BEGIN

/**
 * Abstract parallel process, which performs a certain task (to be defined by
 * the subclass) on the pixels of an image where work on adjacent pixels 
 * is independent. For preview purposes, a spiraling pattern of square 
 * pixel blocks is generated.
 */
class MTS_EXPORT_RENDER BlockedImageProcess : public ParallelProcess {
public:
	/* ParallelProcess interface */
	virtual EStatus generateWork(WorkUnit *unit, int worker);

	MTS_DECLARE_CLASS()
protected:
	/**
	 * Initialize the image process
	 *
	 * @param offset
	 *    Integer offset of the image region to be processed
	 * @param size
	 *    Size of the image region to be processed
	 * @param blockSize
	 *    Size of the generated square pixel blocks
	 */
	void init(const Point2i &offset, const Vector2i &size, int blockSize);

	/// Protected constructor
	inline BlockedImageProcess() { }
	/// Virtual destructor
	virtual ~BlockedImageProcess() { }
protected:
	enum EDirection {
		ERight = 0,
		EDown,
		ELeft,
		EUp
	};

	Point2i m_offset;
	Vector2i m_size, m_numBlocks;
	Point2i m_curBlock;
	int m_direction, m_numSteps;
	int m_stepsLeft, m_numBlocksTotal;
	int m_numBlocksGenerated;
	int m_blockSize;
};

MTS_NAMESPACE_END

#endif /* __IMAGEPROC_H */