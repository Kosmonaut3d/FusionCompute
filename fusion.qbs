import qbs
import qbs.Process
import qbs.File
import qbs.FileInfo
import qbs.TextFile
import "../../../libs/openFrameworksCompiled/project/qtcreator/ofApp.qbs" as ofApp

Project{
    property string of_root: "../../.."

    ofApp {
        name: { return FileInfo.baseName(sourceDirectory) }

        files: [
            "resources/shaders/ICPReduction.comp",
            "resources/shaders/computeICP.comp",
            "resources/shaders/computeICPSDF.comp",
            "resources/shaders/computeSDF.comp",
            "resources/shaders/computeSDFColor.comp",
            "resources/shaders/raymarchSDF.frag",
            "resources/shaders/raymarchSDFColor.frag",
            "src/compute/icpCompute.cpp",
            "src/compute/icpCompute.h",
            'resources/shaders/computeBilateralBlur.comp',
            'resources/shaders/computeModelPCL.comp',
            'resources/shaders/computeNormalPCL.comp',
            'resources/shaders/fragShader.frag',
            'resources/shaders/fullScreenQuad.frag',
            'resources/shaders/fullScreenQuad.vert',
            'resources/shaders/pointCloud.frag',
            'resources/shaders/pointCloud.vert',
            'resources/shaders/sliceFragShader.frag',
            'resources/shaders/vertShader.vert',
            'src/compute/BilateralBlurCompute.cpp',
            'src/compute/BilateralBlurCompute.h',
            'src/compute/PointCloudCompute.cpp',
            'src/compute/PointCloudCompute.h',
            'src/compute/PointCloudVis.cpp',
            'src/compute/PointCloudVis.h',
            'src/compute/sdfCompute.cpp',
            'src/compute/sdfCompute.h',
            'src/cpuReference/IterativeClosestPointCPU.cpp',
            'src/cpuReference/IterativeClosestPointCPU.h',
            'src/cpuReference/PointCloudCPU.cpp',
            'src/cpuReference/PointCloudCPU.h',
            'src/cpuReference/sdf.cpp',
            'src/cpuReference/sdf.h',
            'src/cpuReference/slice.cpp',
            'src/cpuReference/slice.h',
            'src/helper/dataStorageHelper.cpp',
            'src/helper/dataStorageHelper.h',
            'src/helper/eigen2glm.h',
            'src/helper/fullScreenQuadRender.cpp',
            'src/helper/fullScreenQuadRender.h',
            'src/main.cpp',
            'src/ofApp.cpp',
            'src/ofApp.h',
            'src/scenes/GUIScene.cpp',
            'src/scenes/GUIScene.h',
            'src/scenes/PointCloudScene.cpp',
            'src/scenes/PointCloudScene.h',
            'src/scenes/PreprocessDepthScene.cpp',
            'src/scenes/PreprocessDepthScene.h',
            'src/scenes/SDFScene.cpp',
            'src/scenes/SDFScene.h',
        ]

        of.addons: [
            'ofxImGui',
            'ofxKinect',
        ]

        // additional flags for the project. the of module sets some
        // flags by default to add the core libraries, search paths...
        // this flags can be augmented through the following properties:
        of.pkgConfigs: []       // list of additional system pkgs to include
        of.includePaths: []     // include search paths
        of.cFlags: []           // flags passed to the c compiler
        of.cxxFlags: []         // flags passed to the c++ compiler
        of.linkerFlags: []      // flags passed to the linker
        of.defines: []          // defines are passed as -D to the compiler
                                // and can be checked with #ifdef or #if in the code
        of.frameworks: []       // osx only, additional frameworks to link with the project
        of.staticLibraries: []  // static libraries
        of.dynamicLibraries: [] // dynamic libraries

        // other flags can be set through the cpp module: http://doc.qt.io/qbs/cpp-module.html
        // eg: this will enable ccache when compiling
        //
        // cpp.compilerWrapper: 'ccache'

        Depends{
            name: "cpp"
        }

        // common rules that parse the include search paths, core libraries...
        Depends{
            name: "of"
        }

        // dependency with the OF library
        Depends{
            name: "openFrameworks"
        }
    }

    property bool makeOF: true  // use makfiles to compile the OF library
                                // will compile OF only once for all your projects
                                // otherwise compiled per project with qbs
    

    property bool precompileOfMain: false  // precompile ofMain.h
                                           // faster to recompile when including ofMain.h 
                                           // but might use a lot of space per project

    references: [FileInfo.joinPaths(of_root, "/libs/openFrameworksCompiled/project/qtcreator/openFrameworks.qbs")]
}
