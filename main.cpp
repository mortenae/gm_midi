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

#include "main.h"

int references;
int blah;

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	Device* device = reinterpret_cast<Device*>(dwInstance);

	if(wMsg == MIM_DATA)
	{
		Event* event = new Event;

		event->channel = dwParam1 >> 0 & 0x0f;
		event->opcode = dwParam1 >> 4 & 0x07;
		event->data1 = dwParam1 >> 8 & 0x7f;
		event->data2 = dwParam1 >> 16 & 0x7f;
		event->time = dwParam2;

		device->events.push(event);
	}
}

int midi::Open(lua_State* state)
{
	UINT id;
	MMRESULT result;
	Device** device;
	Device* device_object;

	id = (UINT)LUA->CheckNumber(1) - 1;
	device_object = new Device;

	result = midiInOpen(&device_object->handle, id, (DWORD_PTR)&MidiInProc, (DWORD_PTR)device_object, CALLBACK_FUNCTION);

	if(result != MMSYSERR_NOERROR)
	{
		char text[256];

		midiInGetErrorTextA(result, text, sizeof(text));
		LUA->PushNil();
		LUA->PushString(text);
		delete device_object;

		return 2;
	}

	device_object->callback = 0;
	device = (Device**)LUA->NewUserdata(sizeof(Device**));
	*device = device_object;

	LUA->CreateMetaTableType("MIDI", 60299);
	LUA->SetMetaTable(-2);

	LUA->ReferencePush(references);
	LUA->PushNumber(++blah);
	LUA->Push(-3);
	LUA->SetTable(-3);
	LUA->Pop();

	return 1;
}

int midi::GetAll(lua_State* state)
{
	UINT count;

	count = midiInGetNumDevs();
	LUA->CreateTable();

	for(UINT i = 0; i < count; ++i)
	{
		MIDIINCAPS caps;
		midiInGetDevCaps(i, &caps, sizeof(caps));
		LUA->PushNumber(i + 1);
		LUA->PushString(caps.szPname);
		LUA->SetTable(-3);
	}

	return 1;
}

// lua_run_cl require("midi") MIDI = midi.Open(1) midi.Update()

int midi::Update(lua_State* state)
{
	LUA->ReferencePush(references);
	LUA->PushNil();

	while(LUA->Next(-2) != 0)
	{
		Device** device = (Device**)LUA->GetUserdata(-1);

		if(*device)
		{
			if((*device)->callback > 0)
			{
				while(!(*device)->events.empty())
				{
					Event* i = (*device)->events.front();

					LUA->ReferencePush((*device)->callback);
					LUA->Push(-2);
					LUA->PushNumber(i->opcode);
					LUA->PushNumber(i->channel);
					LUA->PushNumber(i->data1);
					LUA->PushNumber(i->data2);
					LUA->PushNumber(i->time / 1000.0);

					if(LUA->PCall(6, 0, 0) != 0)
					{
						LUA->Pop();
					}

					(*device)->events.pop();
				}
			}
		}

		LUA->Pop();
	}

	return 0;
}

#define DEVICE_PROLOGUE() \
	Device** device = (Device**)LUA->GetUserdata(1);\
	if(!device || !*device) LUA->ThrowError("Tried to use a NULL midi device!");

int device::IsValid(lua_State* state)
{
	UINT id;
	Device** device = (Device**)LUA->GetUserdata(1);

	LUA->PushBool(*device != NULL && midiInGetID((*device)->handle, &id) == MMSYSERR_NOERROR);

	return 1;
}

int device::Close(lua_State* state)
{
	DEVICE_PROLOGUE();

	midiInClose((*device)->handle);
	delete *device;
	*device = NULL;

	return 0;
}

int device::Start(lua_State* state)
{
	DEVICE_PROLOGUE();
	midiInStart((*device)->handle);
	return 0;
}

int device::Stop(lua_State* state)
{
	DEVICE_PROLOGUE();
	midiInStop((*device)->handle);
	return 0;
}

int device::Reset(lua_State* state)
{
	DEVICE_PROLOGUE();
	midiInReset((*device)->handle);
	return 0;
}

int device::SetCallback(lua_State* state)
{
	DEVICE_PROLOGUE();

	if((*device)->callback > 0)
	{
		LUA->ReferenceFree((*device)->callback);
		(*device)->callback = 0;
	}

	if(LUA->IsType(2, GarrysMod::Lua::Type::FUNCTION))
	{
		LUA->Push(2);
		(*device)->callback = LUA->ReferenceCreate();
	}

	return 0;
}

int device::Delete(lua_State* state)
{
	Device** device = (Device**)LUA->GetUserdata(1);

	if(*device)
	{
		midiInClose((*device)->handle);
		delete *device;
		*device = NULL;
	}

	return 0;
}

GMOD_MODULE_OPEN()
{
	LUA->CreateMetaTableType("MIDI", 60299);
		LUA->Push(-1);
		LUA->SetField(-2, "__index");

		LUA->PushCFunction(device::Delete);
		LUA->SetField(-2, "__gc");

		LUA->PushCFunction(device::IsValid);
		LUA->SetField(-2, "IsValid");

		LUA->PushCFunction(device::Close);
		LUA->SetField(-2, "Close");

		LUA->PushCFunction(device::SetCallback);
		LUA->SetField(-2, "SetCallback");

		LUA->PushCFunction(device::Start);
		LUA->SetField(-2, "Start");

		LUA->PushCFunction(device::Stop);
		LUA->SetField(-2, "Stop");

		LUA->PushCFunction(device::Reset);
		LUA->SetField(-2, "Reset");
	LUA->Pop();

	LUA->CreateTable();
		LUA->CreateTable();
			LUA->PushString("v");
			LUA->SetField(-2, "__mode");
		LUA->SetMetaTable(-2);
	references = LUA->ReferenceCreate();

	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
		LUA->CreateTable();
			LUA->PushCFunction(midi::Open);
			LUA->SetField(-2, "Open");

			LUA->PushCFunction(midi::GetAll);
			LUA->SetField(-2, "GetAll");

			LUA->PushCFunction(midi::Update);
			LUA->SetField(-2, "Update");

			LUA->PushString(__TIMESTAMP__);
			LUA->SetField(-2, "version");

			LUA->PushNumber(0);
			LUA->SetField(-2, "NOTEOFF");

			LUA->PushNumber(1);
			LUA->SetField(-2, "NOTEON");

			LUA->PushNumber(2);
			LUA->SetField(-2, "AFTERTOUCH");

			LUA->PushNumber(3);
			LUA->SetField(-2, "CONTROL");

			LUA->PushNumber(4);
			LUA->SetField(-2, "PROGRAM");

			LUA->PushNumber(5);
			LUA->SetField(-2, "AFTERTOUCH2");

			LUA->PushNumber(6);
			LUA->SetField(-2, "PITCH");
		LUA->SetField(-2, "midi");

		LUA->GetField(-1, "hook");
			LUA->GetField(-1, "Add");
				LUA->PushString("Think");
				LUA->PushString("gm_midi");
				LUA->PushCFunction(midi::Update);
			LUA->Call(3, 0);
		LUA->Pop();
	LUA->Pop();

	return 0;
}

GMOD_MODULE_CLOSE()
{
	return 0;
}
