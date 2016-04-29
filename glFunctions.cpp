/*
The MIT License (MIT)

ShaderOverride (c) 2016 HappyFunTimes
  derivative of gShaderReplacer (c) 2015 psycholns

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#define LIBRARY_DEF
#include "glfunctions.h"

#include <Windows.h>
#include <sstream>
#include "md5.h"
/*  derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm */

#define PROGRAM_ENV_FOG_INDEX	8
#define PROGRAM_ENV_FOG_ANIM_INDEX 9

class Win32File
{
	HANDLE hFile;
public:
	Win32File( std::string name, DWORD dwAccess, DWORD dwCreationDisposition ) { 
		hFile = CreateFile( name.c_str(), dwAccess, 0, 0, dwCreationDisposition, 0, 0 ); 
	}
	~Win32File() { if( isOpen() ) CloseHandle( hFile ); }

	bool isOpen() { return hFile != INVALID_HANDLE_VALUE; }
	bool write( const char *pointer, unsigned length ) { 
		DWORD t; return isOpen() && WriteFile( hFile, pointer, length, &t, 0 ) != FALSE;
	}
	bool readAll( std::string &contents ) { 
		DWORD t = 0;
		if( isOpen() ) {
			DWORD szFile = GetFileSize( hFile, 0 );
			char *buffer = (char *)HeapAlloc( GetProcessHeap(), 0, szFile );
			if( buffer ) {
				t = ReadFile( hFile, buffer, szFile, &t, 0 );
				if( t != FALSE ) {
					contents.assign( &buffer[0], &buffer[szFile] );
				}

				HeapFree( GetProcessHeap(), 0, buffer );
			}
		}

		return t != 0;
	}
};

void __stdcall my_glProgramString( GLenum target, GLenum format, GLsizei len, const void *string )
{
	std::string hash = md5( std::string( (char*)string, &((char*)string)[len] ) );

	hash.insert( 0, target == GL_FRAGMENT_PROGRAM_ARB ? "fp" : ( target == GL_VERTEX_PROGRAM_ARB ? "vp" : "un" ) );

	CreateDirectory( "shaders_original", 0 );
	Win32File originalFile( "shaders_original/" + hash + ".txt", GENERIC_WRITE, CREATE_ALWAYS );
	originalFile.write( (const char *)string, len );

	CreateDirectory( "shaders_override", 0 );

	Win32File overrideFile( "shaders_override/" + hash + ".txt", GENERIC_READ, OPEN_EXISTING );
	std::string contents;

	if( overrideFile.readAll( contents ) )
		return orig_glProgramString( target, format, contents.length(), contents.c_str() );

	return orig_glProgramString( target, format, len, string );
}

void __stdcall my_glBindProgram( GLenum target, GLuint program )
{
	orig_glBindProgram( target, program );

	if( bFogOn )
		orig_glProgramEnvParameter4d( GL_FRAGMENT_PROGRAM_ARB, PROGRAM_ENV_FOG_INDEX,
			fogBehavior[0], fogBehavior[1], fogBehavior[2], fogBehavior[3] );
	else
		orig_glProgramEnvParameter4d( GL_FRAGMENT_PROGRAM_ARB, PROGRAM_ENV_FOG_INDEX, 0.0, 0.0, 0.0, 1.0 );
}

void fogRecalculate()
{	
	// {DENSITY/LN(2), DENSITY/SQRT(LN(2)), -1/(END-START), END/(END-START)}

	double endStart = fogEnd - fogStart;

	switch( fogMode )
	{
	case GL_LINEAR:
		fogBehavior[0] = 0;
		fogBehavior[1] = 0;
		if( endStart != 0 )
		{
			fogBehavior[2] = -1 / endStart;
			fogBehavior[3] = fogEnd / endStart;
		}
		else
		{
			fogBehavior[2] = 0;
			fogBehavior[3] = 1;
		}
		break;
	case GL_EXP:
		fogBehavior[0] = fogDensity / 0.693147180559945;
		fogBehavior[1] = 0;
		fogBehavior[2] = 0;
		fogBehavior[3] = 1;
		break;
	case GL_EXP2:
		fogBehavior[0] = 0;
		fogBehavior[1] = fogDensity / 0.832554611157697;
		fogBehavior[2] = 0;
		fogBehavior[3] = 1;
		break;
	};

	/*
	PARAM p = program.env[8];
	TEMP fogFactor;
	MUL fogFactor.x, p.x, fragment.fogcoord.x;
	EX2_SAT fogFactor.x, -fogFactor.x;
	MUL fogFactor.y, p.y, fragment.fogcoord.x;
	MUL fogFactor.y, fogFactor.y, fogFactor.y;
	EX2_SAT fogFactor.y, -fogFactor.y;
	MAD_SAT fogFactor.z, p.z, fragment.fogcoord.x, p.w;
	MUL fogFactor.x, fogFactor.y, fogFactor.x;
	MUL fogFactor.x, fogFactor.z, fogFactor.x;
	LRP r0.rgb, fogFactor.x, r0, state.fog.color;
	MOV result.color, r0;
	*/
}