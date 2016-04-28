ShaderOverride by HappyFunTimes copyright (c) 2016
(derivative of gShaderReplacer by psycholns copyright (c) 2015 and
  of the "RSA Data Security, Inc. MD5 Message-Digest Algorithm" by Frank Thilo and RSA Data Security, Inc. copyright (c) 1991-2)
(makes use of 7-Zip copyright (c) 1999-2010 Igor Pavlov in compilation)

See LICENSE_*.txt for licenses.

== Overview
This DLL was produced specifically for Star Wars: Knights of the Old Republic II: The Sith Lords to fix issues in 2016 with the fog.

== Usage
When glProgramSource is called from within a program that uses it, the DLL will produce two directories: "shaders_original" and "shaders_override".  It will then proceed to dump the shaders to the "shaders_original" directory with filenames in the form of <vp/fp><md5_shader_hash>.txt.

The DLL will then check to see if there is a file with a matching name in the "shaders_override" directory and load it instead.  If not, it will use the original shader.

Copy any files that you want to change from original to override and make changes there.  The orignal gets overwritten every run.

== Fog Issue
It seems that the fog is not enabled in certain areas of the game.  As any changes to the executable to glEnable(GL_FOG) could be overwritten by Steam, adding to the shaders to handle fog seems to be the safest route.  This should be able to be copied/pasted into most fragment shaders (fp_______.txt) right before the "MOV result.color, r0" statement:

# == manual interpolation for fog ==
TEMP blendFog;
MAD_SAT blendFog.x, fragment.fogcoord.x, state.fog.params.w, state.fog.params.y;
CMP blendFog.x, -state.fog.color.r, blendFog.x, 0;
LRP r0.rgb, blendFog.x, state.fog.color, r0;

(This code adapted from opengl ARB_vertex_program fog reference implementation.)

I make the assumption that all fog in the game has a non-zero red component to ease handling of fog visibility (like inside the ship, where there is no fog setup - things turn black without that "CMP" line).

== Source Code
Compilation makes use of the 7-Zip program by Igor Pavlov to add a binary resource to the DLL of the source in a 7z archive.  Due to the way 7-Zip handles executables/DLLs, this DLL can simply be opened by the 7-Zip program as if it was an archive to obtain the source code.

When recompiling, you may have to adjust the path in the Pre Build event to the 7-Zip installation directory.

== Bugs
Not aware of any.  If the shaders cause issues, change them.

== Resources
GLIntercept module ShaderEditor
  (https://github.com/dtrebilco/glintercept)
  to identify problem shaders and test new code.
Shader Assembly Language (ARB/NV) Quick Reference Guide for OpenGL®  
  (http://www.renderguild.com/gpuguide.pdf currently)
  good GL shader assembly reference.
ARB_vertex_program
  (http://oss.sgi.com/projects/ogl-sample/registry/ARB/fragment_program.txt)
  fog reference implementations