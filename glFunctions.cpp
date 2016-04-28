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

#include <Windows.h>
#include <string>
#include "glfunctions.h"
#include "md5.h"
/*  derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm */
#include <gl/glew.h>

PFNGLPROGRAMSTRING			orig_glProgramString = NULL;

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

void __stdcall my_glProgramString( unsigned target, unsigned format, unsigned len, const void *pointer )
{
	std::string hash = md5( std::string( (char*)pointer, &((char*)pointer)[len] ) );

	hash.insert( 0, target == GL_FRAGMENT_PROGRAM_ARB ? "fp" : ( target == GL_VERTEX_PROGRAM_ARB ? "vp" : "un" ) );

	CreateDirectory( "shaders_original", 0 );
	Win32File originalFile( "shaders_original/" + hash + ".txt", GENERIC_WRITE, CREATE_ALWAYS );
	originalFile.write( (const char *)pointer, len );

	CreateDirectory( "shaders_override", 0 );

	Win32File overrideFile( "shaders_override/" + hash + ".txt", GENERIC_READ, OPEN_EXISTING );
	std::string contents;

	if( overrideFile.readAll( contents ) )
		return orig_glProgramString( target, format, contents.length(), contents.c_str() );

	return orig_glProgramString( target, format, len, pointer );
}