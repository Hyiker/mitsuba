-- Utilities
includes("utils")
-- Surface scattering models
includes("bsdfs")
-- -- Phase functions
includes("phase")
-- -- Intersection shapes
includes("shapes")
-- -- Sample generators
includes("samplers")
-- -- Reconstruction filters
includes("rfilters")
-- -- Film implementations
includes("films")
-- -- Sensors
includes("sensors")
-- -- Emitters
includes("emitters")
-- -- Participating media
includes("medium")
-- -- Volumetric data sources
includes("volume")
-- -- Sub-surface integrators
includes("subsurface")
-- -- Texture types
includes("textures")
-- -- Integrators
includes("integrators")
-- -- Testcases
includes("tests")

-- move plugin dlls to build directory
set_targetdir("$(targetdir)/plugins")