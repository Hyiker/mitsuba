target("data")
    set_kind("phony")
-- for file in os.listdir(env.GetBuildPath('#data/schema')):
--     if fnmatch.fnmatch(file, '*.xsl') or fnmatch.fnmatch(file, '*.xsd'):
--             installTargets += env.Install(os.path.join(distDir, 'data/schema'), '#data/schema/' + file)
-- for file in os.listdir(env.GetBuildPath('#data/ior')):
--         if fnmatch.fnmatch(file, '*.spd'):
--                 installTargets += env.Install(os.path.join(distDir, 'data/ior'), '#data/ior/' + file)
-- for file in os.listdir(env.GetBuildPath('#data/microfacet')):
--         if fnmatch.fnmatch(file, '*.dat'):
--                 installTargets += env.Install(os.path.join(distDir, 'data/microfacet'), '#data/microfacet/' + file)
    on_build(function (target)
        os.trycp("data/schema", path.join(target:targetdir(), "data/schema"))
        os.trycp("data/ior", path.join(target:targetdir(), "data/ior"))
        os.trycp("data/microfacet", path.join(target:targetdir(), "data/microfacet"))
    end)