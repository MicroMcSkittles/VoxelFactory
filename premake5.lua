workspace "VoxelFactory"
    architecture "x64"
    startproject "VoxelFactory"

    configurations {
        "Debug",
        "Release"
    }

    flags {
        "MultiProcessorCompile"
    }

-- Directory final files will be placed into
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Libraries
group "Dependencies"
group ""

include "VoxelFactory"