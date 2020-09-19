// ----------
// NeoPop Duo
// ----------

#include "duo_instance.h"

DuoInstance *DuoInstance::currentInstance = nullptr;
DuoInstance *DuoInstance::currentInstance = nullptr;
DuoInstance DuoInstance::instances[MAX_INSTANCES];

// ----------------
// DuoTLCS900hState
// ----------------

void DuoTLCS900hState::Capture()
{
	// TODO
}

void DuoTLCS900hState::Restore()
{
	// TODO
}

// -----------
// DuoZ80State
// -----------

void DuoZ80State::Capture()
{
	// TODO
}

void DuoZ80State::Restore()
{
	// TODO
}

// -----------
// DuoInstance
// -----------

DuoInstance::DuoInstance()
{
	this->game = nullptr;
	this->surface = nullptr;
}

DuoInstance::DuoInstance(MDFNGI *game, MDFN_Surface *surface)
{
	this->game = game;
	this->surface = surface;
}

// ------------------
// DuoInstance static
// ------------------

void DuoInstance::StageInstance(DuoInstance *instance)
{
	UnstageCurrentInstance();
	
	if (instance == nullptr)
		return;

	currentInstance = instance;
	currentInstance->tlcs900hState.Restore();
	currentInstance->z80State.Restore();
}

void DuoInstance::UnstageCurrentInstance()
{
	if (currentInstance == nullptr)
		return;

	currentInstance->tlcs900hState.Capture();
	currentInstance->z80State.Capture();
	currentInstance = nullptr;
}

