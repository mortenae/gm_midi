/*
	The MIT License

	Copyright (c) 2013 Morten Erlandsen <animorten@gmail.com>

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
	the Software, and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
	IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
	CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MAIN_H
#define MAIN_H

#define GMMODULE
#define WIN32_LEAN_AND_MEAN

#include <GarrysMod/Lua/Interface.h>
#include <Windows.h>
#include <Mmsystem.h>
#include <queue>

struct Event
{
	unsigned char opcode;
	unsigned char channel;
	unsigned char data1;
	unsigned char data2;
	unsigned long time;
};

struct Device
{
	HMIDIIN handle;
	std::queue<Event*> events;
	int callback;
};

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

namespace midi
{
	int Open(lua_State* state);
	int GetAll(lua_State* state);
	int Update(lua_State* state);
}

namespace device
{
	int IsValid(lua_State* state);
	int Close(lua_State* state);
	int Start(lua_State* state);
	int Stop(lua_State* state);
	int Reset(lua_State* state);
	int SetCallback(lua_State* state);
	int Delete(lua_State* state);
}

#endif
