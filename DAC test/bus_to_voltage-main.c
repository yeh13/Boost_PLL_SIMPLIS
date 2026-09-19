
#line 14 "VaMainFileTemplate.c"
/* Copyright SIMetrix Technologies Ltd. 2026
// File .c
//
// ***WARNING*** This is a generated file. Hand edits will be lost.
//
// *************************************************************************** //
// Model                bus_to_voltage, version 
// Created:             Tue Aug 25 15:41:34 2026
// Source:              C:/Users/Yen Tung Lung/Desktop/DAC test/bus_to_voltage.va
// Template:            VaMainFileTemplate.c
// Template date:       Sat Apr 27 05:26:05 2019
// Compiler version:    2019.0507
// *************************************************************************** // */
#define USE_VA_UTILITIES
#include "interfacelayer.h"
#include <string.h>


/*---------- Macro definitions ---------*/
#if defined(unix)
#define EXPORT
#elif defined(_WIN32)
#define EXPORT __declspec(dllexport)
#endif
//--------------------------------------//


//---------- Type definitions ----------//

//--------------------------------------//


//--------- Extern functions -----------//
//--------------------------------------//


//---------- Extern data ---------------//
//--------------------------------------//


//--------- Local functions ------------//
//--------------------------------------//


//---------- Local Data-----------------//
static SxNatureDefinition _currentPotential = { "$abstol", "A" } ;
static SxNatureDefinition _currentFlow = { "0.0", "" } ;
static SxNatureDefinition _magneticPotential = { "1e-12", "A*turn" } ;
static SxNatureDefinition _magneticFlow = { "$fluxtol", "Wb" } ;
static SxNatureDefinition _rotationalPotential = { "1e-06", "rads" } ;
static SxNatureDefinition _rotationalFlow = { "1e-06", "N*m" } ;
static SxNatureDefinition _kinematic_vPotential = { "1e-06", "m/s" } ;
static SxNatureDefinition _kinematic_vFlow = { "1e-06", "N" } ;
static SxNatureDefinition _kinematicPotential = { "1e-06", "m" } ;
static SxNatureDefinition _kinematicFlow = { "1e-06", "N" } ;
static SxNatureDefinition _rotational_omegaPotential = { "1e-06", "rads/s" } ;
static SxNatureDefinition _rotational_omegaFlow = { "1e-06", "N*m" } ;
static SxNatureDefinition _voltagePotential = { "$vntol", "V" } ;
static SxNatureDefinition _voltageFlow = { "0.0", "" } ;
static SxNatureDefinition _thermalPotential = { "0.0001", "K" } ;
static SxNatureDefinition _thermalFlow = { "1e-09", "W" } ;
static SxNatureDefinition _electricalPotential = { "$vntol", "V" } ;
static SxNatureDefinition _electricalFlow = { "$abstol", "A" } ;
#line 64 

static SxDisciplineDefinition disciplines[] = 
{
    { "current", "signal_flow", &_currentPotential, &_currentFlow, sizeof(SxNatureDefinition) },
    { "magnetic", "conservative", &_magneticPotential, &_magneticFlow, sizeof(SxNatureDefinition) },
    { "rotational", "conservative", &_rotationalPotential, &_rotationalFlow, sizeof(SxNatureDefinition) },
    { "kinematic_v", "conservative", &_kinematic_vPotential, &_kinematic_vFlow, sizeof(SxNatureDefinition) },
    { "kinematic", "conservative", &_kinematicPotential, &_kinematicFlow, sizeof(SxNatureDefinition) },
    { "rotational_omega", "conservative", &_rotational_omegaPotential, &_rotational_omegaFlow, sizeof(SxNatureDefinition) },
    { "voltage", "signal_flow", &_voltagePotential, &_voltageFlow, sizeof(SxNatureDefinition) },
    { "thermal", "conservative", &_thermalPotential, &_thermalFlow, sizeof(SxNatureDefinition) },
    { "electrical", "conservative", &_electricalPotential, &_electricalFlow, sizeof(SxNatureDefinition) },
#line 71 
} ;

static SxModuleInfo moduleInfo[] =
{
    { "bus_to_voltage", 1, 0, 0, 0 },
#line 79 
} ;

static SxCompositeDefinition compositeDefinition =
{
    "C:/Users/Yen Tung Lung/Desktop/DAC test/bus_to_voltage.va",

    disciplines,
    sizeof(disciplines)/sizeof(*disciplines),

    moduleInfo,
    sizeof(moduleInfo)/sizeof(*moduleInfo) 
} ;

//--------------------------------------//


//---------- Public data ---------------//
SxdevVaSupportFunctions vaSupportFunctions ;
//--------------------------------------//

EXPORT void ild_GetSourceId(char **id)
{
	*id = "bus_to_voltage";
}


EXPORT int ild_LoadVaSupportFunctions(SxdevVaSupportFunctions *invaSupportFunctions)
{
	int minsize = invaSupportFunctions->sizeOfThisObject ;
	if (minsize>sizeof(SxdevVaSupportFunctions))
		minsize = sizeof(SxdevVaSupportFunctions) ;
	 
	memcpy(&vaSupportFunctions, invaSupportFunctions, minsize) ; 
    return 0 ;
}


    


// This is the main 'C' file
EXPORT void ild_OpenDefinition(SxCompositeDefinition **pcompositeDefinition)
{
    *pcompositeDefinition = &compositeDefinition ;
}

EXPORT void ild_GetSourceCheckSum(char **checkSum, char **ctParams)
{
	*checkSum = "4C40FD9E1294C20EBD2F7AEFF32DC764" ;
	*ctParams = "" ;
}

#if defined(_WIN32)
// some compilers require this - Digital Mars for example
int __stdcall DllMain(
  void *hinstDLL,  // handle to the DLL module
  unsigned fdwReason,     // reason for calling function
  void * lpvReserved   // reserved
)
{
	return 1 ;
}
#endif
