[TOC]

# Directories
## Root
The root directory is the base of the project. Many important files relating to project configuration are located here. The ```.ioc``` file is used to generate configuration code in STM32CubeMX, the ```.launch``` files configure debugging parameters, and the ```.project``` file defines the project configuration in STM32CubeIDE. 

> [!WARNING]
> Many of the configuration files are particular about directory and files names. You will most likely need to rename the .ioc file, and create a new launch configuration when cloning the repository from Github.

## Core/Src
The Src (or source) directory contains most of the operational code of the CanFest implementation.
Specific details of each of the files can be found under Topics->CANFestival.

This directory also contains main.c, which contains the main initialization code of the module, configuring system clocks,
peripherals used for communication with the CAN network, and peripherals used for internal state management.

The main file in this directory that will be modified for user implemtation is @ref nmtSlave.c, which defines functionality on reception of a specific NMT command.

## Core/Inc
The Inc (or include) directory contains the header files for files within the Core/Src directory.

The main file in this directory that will be modified for user implementation is @ref def.h, which defines NMT commands the module is able to receive.

## Core/app
The app directory contains the user application files. This is where your custom implementation of the module will live.

### module Folder
There are a few files present in the directory that are required for operation of the generic module, but are not part of the core CanFest implentation, and thus, are included here.

These files are @ref acceltemp.c "acceltemp.c/h" and @ref eedata.c "eedata.c/h", which are the parts of the program which manage communication with the accelerometer, thermometer, diagnostics measurements, and emulated EEPROM. These files must not be changed unless you know what you're doing.

> [!WARNING]
> This is particilarly true for the emulated EEPROM, whose configuration and functionality is linked to, and must match the configuration in the module bootloader.

### Object Dictionary
@ref ObjDict.c "ObjDict.c/h" are also in this directory, which defines the Object Dictionary from which most of the data communication with FESCAN will take place.

# Restricted Peripherals
There are a few peripherals that are restricted from use in a custom module implementation due to use in the core module functonality.
These peripherals are ADC1*, CAN1, TIM2, I2C2, and RCC. 

| Peripheral | Reason       |
| ---------- | ------------ |
| ADC1*      | ADC1 is used to measure internal voltages for diagnostics reasons. ADC1 is able to be used if the diagnostics channels are initialized again when additional ADC channels need to be used (as is done in SWARM). If possible, an external ADC is recommended. |
| CAN1       | The CAN interface is used for communication with the broader network. |
| TIM2       | The Timer2 peripheral is used as the timebase for internally timing the CanFest layer. |
| I2C2       | The I2C2 interface is used to communicate with the accelerometer and thermometer on pins PB13 and PB14. |
| RCC        | Changing the system clocks requires retiming the TIM2, CAN, I2C, and CanFest, and is not recommended. |

> [!NOTE]
> If for some reason the user application returns to the bootloader without resetting, the RTC must be deinitialized, or the CAN interface has a significant probability of not working.

# Configuration Differences
There are several notable differences between a project generated by STM32CubeIDE and the Generic Remote Module, all relating to the memory mapping of the module.

## Core/Src/system_stm32l4xx.c
The following changes are made to the @ref system_stm32l4xx.c file:
- ```#define USER_VECT_TAB_ADDRESS``` - is defined
- ```#define VECT_TAB_OFFSET 0x00005000U``` - changed from 0x00000000U

These changes enable the implementation of the custom CAN bootloader.

## STM32L433RCTXP_FLASH.ld
This is the linker script that defines the addressing of FLASH, RAM, and other important addresses of the microcontroller.

The only change is made to the *Memories definiton* section.

```c
MEMORY
{
  RAM    (xrw)    : ORIGIN = 0x20000000,   LENGTH = 48K
  RAM2    (xrw)    : ORIGIN = 0x10000000,   LENGTH = 16K
  FLASH    (rx)    : ORIGIN = 0x08005000,   LENGTH = 232K
}
```

The flash origin has a 0x5000 offset due to the bootloader, and the length is shortened, due to the bootloader, module parameter storage, and reserved EEPROM space.

For a 256K module:
- 256K - 20K (bootloader) - 2K (parameter space) - 2K (EEPROM) = 232K

# Object Dictionary
The Object Dictionary is the primary index for reading and writing data via FESCAN/CanFest. The @ref ObjDict.c "Object Dictionary" conatins module configuration data, state information, sensor information, etc; and is where any data you would like to retrieve over FESCAN will live. 

## Default/Common Entries
There are several entries in the Object Dictionary which are consistent across all FESCAN/CanFest devices. A non-exhaustive list of entries and their description are included below:

| Entry Index | Entry Description | 
| ----------- | ---------- |
| 0x1000 - 0x2000 | CanFest Communication Parameters | 
| 0x2003 | Temperature |
| 0x2010 | Module Status |
| 0x2011 | Accelerometer Data |
| 0x2500 | CAN Statuses | 
| 0x2900 | Restore List |
| 0x3000 | Diagnostics |

## Adding Entries
Adding entries to the object dictionary is fairly simple. As an example, we will walk through adding two new entries to the Object Dictionary, SimpleVariable, and SimpleArray.

### Creating Variables
At the top of @ref ObjDict.c (located in Core/App/), there is a list of mapped variables. To ensure compatibility with the network, mapped entries should use the built in variable types (@ref UNS8, @ref UNS16, @ref UNS32, @ref INTEGER8, @ref INTEGER16, etc...).

To begin, we will add SimpleVariable (a @ref UNS8) and SimpleArray (a @ref UNS8[16]) to the list. Variables that are included in the Object Dictionary must have a default value assigned to them. This value can be updated within the application at any point.

```c
/* ObjDict.c */
...
UNS8 Diagnostic_VOS = 0x00;
UNS8 Diagnostic_VDD = 0x00;

UNS8 SimpleVariable = 0;
UNS8 SimpleArray[16] = 
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
```

In order for other parts of the program to be able to access these new variables we created, we need to add them to ObjDict.h as an ```extern``` variable.

```c
/* ObjDict.h */
...
extern UNS8 Diagnostic_VOS;
extern UNS8 Diagnostic_VDD;

extern UNS8 SimpleVariable;
extern UNS8 SimpleArray[16];
```

### Defining Object Dictionary Structures
With the new entries in the Object Dictionary created, we can now add them into the Object Dictionary itself. Scrolling through the Object Dictionary, you will see the repetition of a specific structure. The one for Accelerometer data is included below:

```c
/* ObjDict.c */
/* index 0x2011 :   Mapped variable Accelerometer Values */
                    UNS8 ObjDict_highestSubIndex_obj2011 = 4; /* number of subindex - 1*/
                    const subindex ObjDict_Index2011[] =
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&ObjDict_highestSubIndex_obj2011 },
                       { RW, uint8, 4, (void*)&Accelerometers[0] },
                       { RW, uint8, 4, (void*) &AccelerometersFiltered[0] },
                       { RW, int8, 4, (void*) &AccelerometersTilt[0] },
                       { RO, uint8, 16, (void*) &Quaternion[0] }
                     };
```

Indexes and their metadata in the Object Dictionary have a distinct naming structure:

‎```ObjDict_IndexXYZW[]``` & ```ObjDict_highestSubIndex_objXYZW```,

where XYZW is the index address of the mapped variables. 

> [!WARNING]
> Care should be taken to ensure that indexes do not overlap with other module types in the network, this is a preventative measure to ensure mismatched data assignment does not occur between two types of modules.
>
> e.g, Writing data meant for a Pulse Generator module to a Biopotential module.

We will create a new entry in the Object Dictionary following the same structure. In this case we will include both variables we created in the same index, but they can easily be split up into different indices if desired.

> [!TIP]
> To keep things organized, variables should be grouped together by function.

For no particular reason, we will create the new entry at index 0xBEEF.

```c
/* index 0xBEEF :   Mapped variable Simple Test Variables */
                    UNS8 ObjDict_highestSubIndex_objBEEF = 2; /* number of subindex - 1*/
                    const subindex ObjDict_IndexBEEF[] =
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&ObjDict_highestSubIndex_objBEEF },
                       { RW, uint8, sizeof (UNS8), (void*)&SimpleVariable },
                       { RW, uint8, sizeof(SimpleArray), (void*)&SimpleArray[0] }
                     };
```

- ```ObjDict_highestSubIndex_objBEEF``` defines how many subindices are present in the given index, and is equal to the number of variables you would like to include.
    - The first subindex in an index structure will always be the subindex count.

Lets break down the different parts in a @ref td_subindex "subindex":
- Variables in an subindex can be assigned as @ref RO "Read Only", @ref RW "Read Write", or @ref WO "Write Only" across the CAN network, this is useful for something like ADC data, which should only be written to by the module itself, or for our SimpleVariable example, which can be written to or read from.
- The next part describes the datatype of the variable, a list of different data types are available in @ref objdictdef.h.
- The next part describes the size of the object in the Object Dictionary. FESCAN  has a limit of 48 bytes that can be read across the network in a given request.
- The last part describes the object itself. Objects in the Object Dictionary are referenced to by their address. In our case, these are pointers to the SimpleVariable and SimpleArray variables.

> [!NOTE]
> To add an array to the Object Dictionary, the size of the object will be ```sizeof(array)```, but the pointer to the object will be a reference to the 0th index of the array.

### Registering Object Dictionary Entries
Now that the Object Dictionary structure is defined, we can register the Object Dictionary entry in the @ref indextable.

Near the end of @ref ObjDict.c, there will be a large array named ObjDict_objdict, this is the @ref indextable of our Object Dictionary.

To register our new index in the Object Dictonary, we can simply add our new entry to the end of the list.

```c
...
  { (subindex*)ObjDict_Index2020,sizeof(ObjDict_Index2020)/sizeof(ObjDict_Index2020[0]), 0x2020},
  { (subindex*)ObjDict_Index2500,sizeof(ObjDict_Index2500)/sizeof(ObjDict_Index2500[0]), 0x2500},
  { (subindex*)ObjDict_Index2900,sizeof(ObjDict_Index2900)/sizeof(ObjDict_Index2900[0]), 0x2900},
  { (subindex*)ObjDict_Index3000,sizeof(ObjDict_Index3000)/sizeof(ObjDict_Index3000[0]), 0x3000},
  { (subindex*)ObjDict_IndexBEEF,sizeof(ObjDict_IndexBEEF)/sizeof(ObjDict_IndexBEEF[0]), 0xBEEF}
};
```

Additionally, the index must be added to the search function ```ObjDict_scanIndexOD()```.

```c
...
        case 0x2020: i = 18;break;
        case 0x2500: i = 19;break;
        case 0x2900: i = 20;break;
        case 0x3000: i = 21;break;
        case 0xBEEF: i = 22;break;
		default:
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
	}
	*errorCode = OD_SUCCESSFUL;
	return &ObjDict_objdict[i];
}
```
> [!IMPORTANT]
> Ensure that the index of your new entry in ```ObjDict_scanIndexOD()``` aligns with the index in ```ObjDict_objdict[]```.

<h2>Congratulations! You have added new entries to the Object Dictionary that can be read across the FESCAN network! </h2>

> [!NOTE]
> This can be tested by using *NNPHELPERS* in Matlab: \n
> ``` nnp.write(node, 'BEEF', 1, 42, 'uint8');```
> ``` nnp.read(node, 'BEEF', 1);```
> ``` nnp.read(node, 'BEEF', 2);```


## Adding Callbacks
Callbacks can be added to specific subindices to call a function when modified in the Object Dictionary via FESCAN.
This is useful if a specific variable is used for asynchronous code execution, or as an initialization parameter.

Adding a callback is fairly straightforward, and only requires slight modifications to our existing structures. We will continue from our SimpleVariable example above.

The first modification is at our ```0xBEEF``` index:

```c
/* index 0xBEEF :   Mapped variable Simple Test Variables */
                    UNS8 ObjDict_highestSubIndex_objBEEF = 2; /* number of subindex - 1*/
                    ODCallback_t ObjDict_IndexBEEF_callbacks[] =
                     {
                       NULL,
                       NULL,
                       NULL
                     };
                    const subindex ObjDict_IndexBEEF[] =
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&ObjDict_highestSubIndex_objBEEF },
                       { RW, uint8, sizeof (UNS8), (void*)&SimpleVariable },
                       { RW, uint8, sizeof(SimpleArray), (void*)&SimpleArray[0] }
                     };
```
We create a new array ```ObjDict_IndexBEEF_callbacks[]``` of type ```ODCallback_t```, that contains as many ```NULL``` elements as we have in our subindex table (including subIndex count).

In ```ObjDict_scanIndexOD``` we add the set of callbacks to the end of the case statement, so instead of 

```c
...
        case 0x2020: i = 18;break;
        case 0x2500: i = 19;break;
        case 0x2900: i = 20;break;
        case 0x3000: i = 21;break;
        case 0xBEEF: i = 22;break;
		default:
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
	}
	*errorCode = OD_SUCCESSFUL;
	return &ObjDict_objdict[i];
}
```

like we had before, the table entry will look like this:

```c
...
        case 0x2020: i = 18;break;
        case 0x2500: i = 19;break;
        case 0x2900: i = 20;break;
        case 0x3000: i = 21;break;
        case 0xBEEF: i = 22;*callbacks = ObjDict_IndexBEEF_callbacks;break;
		default:
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
	}
	*errorCode = OD_SUCCESSFUL;
	return &ObjDict_objdict[i];
}
```

### Registering an Object Dictionary Callback to a Function
Now that our index in the Object Dictionary has slots for callback functions assigned, we can assign a callback function to it.

All callback functions for the object dictionary must have the following structure:

```c
UNS32 FunctionName(CO_Data* d, const indextable * pIndexTable, UNS8 bSubindex)
{
    /* Code Execution */

	return ReturnValue;
}
```

In our case, we will add the following function to @ref app.c:

```c
UNS32 SimpleCallback(CO_Data* d, const indextable * unused_indextable, UNS8 unused_bSubindex)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, !!SimpleVariable); /* Set PB15 to SimpleVariable */

	return 0;
}
```

and the following line to @ref app.h:

```c
UNS32 SimpleCallback(CO_Data* d, const indextable * unused_indextable, UNS8 unused_bSubindex);
```
> [!NOTE]
> If multiple entries in the Object Dictionary use the same callback function, the indexTable and bSubindex parameters can be used to differentiate them.

In @ref initAppTask() we call the following function to register a change in SimpleVariable to our newly created function.

```c
RegisterSetODentryCallBack(&ObjDict_Data, 0xBEEF, 0x01, &SimpleCallback);
```

<h2>Congratulations! You have added a callback that runs when a variable in the Object Dictionary is updated! </h2>

## Add to Restore List
Indices in the Object Dictionary can be restored if they are included in the RestoreList entry in the ObjectDictionary (0x2900).
A given state of the Object Dictionary can be saved via the @ref SaveValues() function, or over the CAN network via the NMT_Do_Save_Cmd NMT command.

We will add our example index ```0xBEEF``` to the restore list.

```c
/* ObjDict.c */
UNS16 RestoreList[RESTORE_COUNT] =  { 0x1400, /*RPDO Params(10)*/ \
                                      0x1600, /*RPDO Mapping(32)*/ \
                                      0x1800, /*TPDO Params(10)*/ \
                                      0x1A00, /*TPDO Mapping(32)*/ \
                                      0x2001, /*Control(4)*/\
                                      0x2012, /*Accelerometer Settings*/\
                                      0xBEEF  /*SimpleVariable/Array*/};
```

> [!WARNING]
> Ensure that the RESTORE_COUNT in ```ObjDict.h``` lines up with the number of indices in ```RestoreList```, otherwise, you will get a compilation error

# Adding Functionality
Application functionality is added via source/header files within the Core/app directory. Instead of a main.c, the module application code should live in @ref app.c.

Initialization code takes place in @ref initAppTask(), and the continuously running loop takes place in @ref updateAppTask().

There are also several useful callbacks present in @ref app.c that are linked to FESCAN/CanFest functionality.

- @ref processSYNCMessageForApp() - Called when a SYNC message is received, check documentation for details.
- @ref sleepApplication() -  Allows the user application to put any peripherals into a low power state before the module enters a low power state.
- @ref InitAppTaskValues() - Reinitializes user application values, check documentation for details.

State management callbacks are also available for user application modification.

> [!IMPORTANT]
> Do not modify main.c, outside of changes from modifying STM_RM.ioc. Application code should live in Core/app

An example of implemented application code is locally present in @ref acceltemp.c, with a full application example in SWARM.

## STM_RM.ioc
‎```STM_RM.ioc``` is the STM32CubeMX configuration file that enables creation of initialization code for pin function and system clocks through a GUI.

As long as the existing set up peripherals and pin assignments are not modified, the ioc file can be used to add additional peripherals and GPIO to the project.

> [!NOTE]
> STM_RM.ioc must be renamed to match the name of the folder that contains it. 