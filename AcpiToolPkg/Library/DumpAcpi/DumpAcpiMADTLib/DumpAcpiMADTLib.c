/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DumpAcpiTableFuncLib.h>
#include <IndustryStandard/Acpi.h>

#pragma pack(1)

typedef struct {
  UINT8                 Type;
  UINT8                 Length;
} APIC_STRUCT_HEADER;

#pragma pack()

VOID
DumpApicProcessorLocalApic (
  EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC_STRUCTURE           *ProcessorLocalApic
  )
{
  if (ProcessorLocalApic == NULL) {
    return;
  }
  
  Print (         
    L"  ***************************************************************************\n"
    L"  *       Processor Local APIC Structure                                    *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Processor Local APIC address ........................... 0x%016lx\n" :
    L"  Processor Local APIC address ........................... 0x%08x\n",
    ProcessorLocalApic
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    ProcessorLocalApic->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    ProcessorLocalApic->Length
    );
  Print (
    L"    ACPI Processor ID .................................... 0x%02x\n",
    ProcessorLocalApic->AcpiProcessorId
    );
  Print (
    L"    APIC ID .............................................. 0x%02x\n",
    ProcessorLocalApic->ApicId
    );
  Print (
    L"    Flags ................................................ 0x%08x\n",
    ProcessorLocalApic->Flags
    );
  Print (
    L"      Enabled ............................................ 0x%08x\n",
    ProcessorLocalApic->Flags & EFI_ACPI_1_0_LOCAL_APIC_ENABLED
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicIOApic (
  EFI_ACPI_1_0_IO_APIC_STRUCTURE                        *IOApic
  )
{
  if (IOApic == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IO APIC Structure                                                 *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  IO APIC address ........................................ 0x%016lx\n" :
    L"  IO APIC address ........................................ 0x%08x\n",
    IOApic
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    IOApic->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    IOApic->Length
    );
  Print (
    L"    IO APIC ID ........................................... 0x%02x\n",
    IOApic->IoApicId
    );
  Print (
    L"    IO APIC Address ...................................... 0x%08x\n",
    IOApic->IoApicAddress
    );
  Print (
    L"    Global System Interrupt Base ......................... 0x%08x\n",
    IOApic->SystemVectorBase
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicInterruptSourceOverride (
  EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE      *InterruptSourceOverride
  )
{
  if (InterruptSourceOverride == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Interrupt Source Override Structure                               *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Interrupt Source Override address ...................... 0x%016lx\n" :
    L"  Interrupt Source Override address ...................... 0x%08x\n",
    InterruptSourceOverride
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    InterruptSourceOverride->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    InterruptSourceOverride->Length
    );
  Print (
    L"    Bus .................................................. 0x%02x\n",
    InterruptSourceOverride->Bus
    );
  Print (
    L"    Source ............................................... 0x%02x\n",
    InterruptSourceOverride->Source
    );
  Print (
    L"    Global System Interrupt .............................. 0x%08x\n",
    InterruptSourceOverride->GlobalSystemInterruptVector
    );
  Print (
    L"    Flags ................................................ 0x%04x\n",
    InterruptSourceOverride->Flags
    );
  Print (
    L"      Polarity ........................................... 0x%04x\n",
    InterruptSourceOverride->Flags & 0x3
    );
  switch (InterruptSourceOverride->Flags & 0x3) {
  case 0x0:
    Print (
     L"        Conforms to the specifications of the bus\n"
     );
    break;
  case 0x1:
    Print (
     L"        Active high\n"
     );
    break;
  case 0x3:
    Print (
     L"        Active low\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"      Trigger Mode ....................................... 0x%04x\n",
    InterruptSourceOverride->Flags & 0xc
    );
  switch (InterruptSourceOverride->Flags & 0xc) {
  case 0x0:
    Print (
     L"        Conforms to the specifications of the bus\n"
     );
    break;
  case 0x4:
    Print (
     L"        Edge-triggered\n"
     );
    break;
  case 0xc:
    Print (
     L"        Level-triggered\n"
     );
    break;
  default:
    break;
  }
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicNonMaskableInterruptSource (
  EFI_ACPI_1_0_NON_MASKABLE_INTERRUPT_SOURCE_STRUCTURE  *NonMaskableInterruptSource
  )
{
  if (NonMaskableInterruptSource == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Non-Maskable Interrupt Source Structure                           *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  NMI Source address ..................................... 0x%016lx\n" :
    L"  NMI Source address ..................................... 0x%08x\n",
    NonMaskableInterruptSource
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    NonMaskableInterruptSource->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    NonMaskableInterruptSource->Length
    );
  Print (
    L"    Flags ................................................ 0x%04x\n",
    NonMaskableInterruptSource->Flags
    );
  Print (
    L"      Polarity ........................................... 0x%04x\n",
    NonMaskableInterruptSource->Flags & 0x3
    );
  switch (NonMaskableInterruptSource->Flags & 0x3) {
  case 0x0:
    Print (
     L"        Conforms to the specifications of the bus\n"
     );
    break;
  case 0x1:
    Print (
     L"        Active high\n"
     );
    break;
  case 0x3:
    Print (
     L"        Active low\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"      Trigger Mode ....................................... 0x%04x\n",
    NonMaskableInterruptSource->Flags & 0xc
    );
  switch (NonMaskableInterruptSource->Flags & 0xc) {
  case 0x0:
    Print (
     L"        Conforms to the specifications of the bus\n"
     );
    break;
  case 0x4:
    Print (
     L"        Edge-triggered\n"
     );
    break;
  case 0xc:
    Print (
     L"        Level-triggered\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"    Global System Interrupt .............................. 0x%08x\n",
    NonMaskableInterruptSource->GlobalSystemInterruptVector
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicLocalApicNMI (
  EFI_ACPI_1_0_LOCAL_APIC_NMI_STRUCTURE                 *LocalApicNMI
  )
{
  UINT16               Flags;
  
  if (LocalApicNMI == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Local APIC NMI Structure                                          *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Local APIC NMI address ................................. 0x%016lx\n" :
    L"  Local APIC NMI address ................................. 0x%08x\n",
    LocalApicNMI
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    LocalApicNMI->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    LocalApicNMI->Length
    );
  Print (
    L"    ACPI Processor ID .................................... 0x%02x\n",
    LocalApicNMI->AcpiProcessorId
    );
  CopyMem (&Flags, &LocalApicNMI->Flags, sizeof(UINT16));
  Print (
    L"    Flags ................................................ 0x%04x\n",
    Flags
    );
  Print (
    L"      Polarity ........................................... 0x%04x\n",
    Flags & 0x3
    );
  switch (Flags & 0x3) {
  case 0x0:
    Print (
     L"        Conforms to the specifications of the bus\n"
     );
    break;
  case 0x1:
    Print (
     L"        Active high\n"
     );
    break;
  case 0x3:
    Print (
     L"        Active low\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"      Trigger Mode ....................................... 0x%04x\n",
    Flags & 0xc
    );
  switch (Flags & 0xc) {
  case 0x0:
    Print (
     L"        Conforms to the specifications of the bus\n"
     );
    break;
  case 0x4:
    Print (
     L"        Edge-triggered\n"
     );
    break;
  case 0xc:
    Print (
     L"        Level-triggered\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"    Local APIC INTI ...................................... 0x%02x\n",
    LocalApicNMI->LocalApicInti
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicLocalApicAddressOverride (
  EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE_STRUCTURE    *LocalApicAddressOverride
  )
{
  UINT64               LocalApicAddress;
  
  if (LocalApicAddressOverride == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Local APIC Address Override Structure                             *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Local APIC Address Override address .................... 0x%016lx\n" :
    L"  Local APIC Address Override address .................... 0x%08x\n",
    LocalApicAddressOverride
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    LocalApicAddressOverride->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    LocalApicAddressOverride->Length
    );
  CopyMem (&LocalApicAddress, &LocalApicAddressOverride->LocalApicAddress, sizeof(UINT64));
  Print (
    L"    Local APIC Address ................................... 0x%016lx\n",
    LocalApicAddress
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicIOSapic (
  EFI_ACPI_2_0_IO_SAPIC_STRUCTURE                       *IOSapic
  )
{
  if (IOSapic == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IO SAPIC Structure                                                *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  IO SAPIC address ....................................... 0x%016lx\n" :
    L"  IO SAPIC address ....................................... 0x%08x\n",
    IOSapic
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    IOSapic->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    IOSapic->Length
    );
  Print (
    L"    IO SAPIC ID .......................................... 0x%02x\n",
    IOSapic->IoApicId
    );
  Print (
    L"    Global System Interrupt Base ......................... 0x%08x\n",
    IOSapic->GlobalSystemInterruptBase
    );
  Print (
    L"    IO SAPIC Address ..................................... 0x%016lx\n",
    IOSapic->IoSapicAddress
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicProcessorLocalSapic (
  EFI_ACPI_2_0_PROCESSOR_LOCAL_SAPIC_STRUCTURE          *ProcessorLocalSapic
  )
{
  UINT32               Flags;

  if (ProcessorLocalSapic == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Processor Local SAPIC Structure                                   *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Processor Local SAPIC address .......................... 0x%016lx\n" :
    L"  Processor Local SAPIC address .......................... 0x%08x\n",
    ProcessorLocalSapic
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    ProcessorLocalSapic->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    ProcessorLocalSapic->Length
    );
  Print (
    L"    ACPI Processor ID .................................... 0x%02x\n",
    ProcessorLocalSapic->AcpiProcessorId
    );
  Print (
    L"    Local SAPIC ID ....................................... 0x%02x\n",
    ProcessorLocalSapic->LocalSapicId
    );
  Print (
    L"    Local SAPIC EID ...................................... 0x%02x\n",
    ProcessorLocalSapic->LocalSapicEid
    );
  CopyMem (&Flags, &ProcessorLocalSapic->Flags, sizeof(UINT32));
  Print (
    L"    Flags ................................................ 0x%08x\n",
    Flags
    );
  Print (
    L"      Enabled ............................................ 0x%08x\n",
    Flags & EFI_ACPI_2_0_LOCAL_APIC_ENABLED
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicPlatformInterruptSource (
  EFI_ACPI_2_0_PLATFORM_INTERRUPT_SOURCES_STRUCTURE      *PlatformInterruptSource
  )
{
  if (PlatformInterruptSource == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Platform Interrupt Source Structure                               *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Platform Interrupt Source address ...................... 0x%016lx\n" :
    L"  Platform Interrupt Source address ...................... 0x%08x\n",
    PlatformInterruptSource
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    PlatformInterruptSource->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    PlatformInterruptSource->Length
    );
  Print (
    L"    Flags ................................................ 0x%04x\n",
    PlatformInterruptSource->Flags
    );
  Print (
    L"      Polarity ........................................... 0x%04x\n",
    PlatformInterruptSource->Flags & 0x3
    );
  switch (PlatformInterruptSource->Flags & 0x3) {
  case 0x0:
    Print (
     L"        Conforms to the specifications of the bus\n"
     );
    break;
  case 0x1:
    Print (
     L"        Active high\n"
     );
    break;
  case 0x3:
    Print (
     L"        Active low\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"      Trigger Mode ....................................... 0x%04x\n",
    PlatformInterruptSource->Flags & 0xc
    );
  switch (PlatformInterruptSource->Flags & 0xc) {
  case 0x0:
    Print (
     L"        Conforms to the specifications of the bus\n"
     );
    break;
  case 0x4:
    Print (
     L"        Edge-triggered\n"
     );
    break;
  case 0xc:
    Print (
     L"        Level-triggered\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"    Interrupt Type ....................................... 0x%02x\n",
    PlatformInterruptSource->InterruptType
    );
  switch (PlatformInterruptSource->InterruptType) {
  case 1:
    Print (
      L"      PMI\n"
      );
    break;
  case 2:
    Print (
      L"      INIT\n"
      );
    break;
  case 3:
    Print (
      L"      Corrected Platform Error Interrupt\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    Processor ID ......................................... 0x%02x\n",
    PlatformInterruptSource->ProcessorId
    );
  Print (
    L"    Processor EID ........................................ 0x%02x\n",
    PlatformInterruptSource->ProcessorEid
    );
  Print (
    L"    IO SAPIC Vector ...................................... 0x%02x\n",
    PlatformInterruptSource->IoSapicVector
    );
  Print (
    L"    Global System Interrupt .............................. 0x%08x\n",
    PlatformInterruptSource->GlobalSystemInterrupt
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicProcessorLocalX2Apic (
  EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE           *ProcessorLocalX2Apic
  )
{
  if (ProcessorLocalX2Apic == NULL) {
    return;
  }
  
  Print (         
    L"  ***************************************************************************\n"
    L"  *       Processor Local X2APIC Structure                                  *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Processor Local X2APIC address ......................... 0x%016lx\n" :
    L"  Processor Local X2APIC address ......................... 0x%08x\n",
    ProcessorLocalX2Apic
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    ProcessorLocalX2Apic->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    ProcessorLocalX2Apic->Length
    );
  Print (
    L"    X2ACPI ID ............................................ 0x%08x\n",
    ProcessorLocalX2Apic->X2ApicId
    );
  Print (
    L"    Flags ................................................ 0x%08x\n",
    ProcessorLocalX2Apic->Flags
    );
  Print (
    L"      Enabled ............................................ 0x%08x\n",
    ProcessorLocalX2Apic->Flags & EFI_ACPI_1_0_LOCAL_APIC_ENABLED
    );
  Print (
    L"    ACPI Processor UID ................................... 0x%08x\n",
    ProcessorLocalX2Apic->AcpiProcessorUid
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicLocalX2ApicNmi (
  EFI_ACPI_4_0_LOCAL_X2APIC_NMI_STRUCTURE                 *LocalX2ApicNmi
  )
{
  UINT16               Flags;

  if (LocalX2ApicNmi == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Local X2APIC NMI Structure                                        *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Local X2APIC NMI address ............................... 0x%016lx\n" :
    L"  Local X2APIC NMI address ............................... 0x%08x\n",
    LocalX2ApicNmi
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    LocalX2ApicNmi->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    LocalX2ApicNmi->Length
    );
  CopyMem (&Flags, &LocalX2ApicNmi->Flags, sizeof(UINT16));
  Print (
    L"    Flags ................................................ 0x%04x\n",
    Flags
    );
  Print (
    L"      Polarity ........................................... 0x%04x\n",
    Flags & 0x3
    );
  switch (Flags & 0x3) {
  case 0x0:
    Print (
     L"        Conforms to the specifications of the bus\n"
     );
    break;
  case 0x1:
    Print (
     L"        Active high\n"
     );
    break;
  case 0x3:
    Print (
     L"        Active low\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"      Trigger Mode ....................................... 0x%04x\n",
    Flags & 0xc
    );
  switch (Flags & 0xc) {
  case 0x0:
    Print (
     L"        Conforms to the specifications of the bus\n"
     );
    break;
  case 0x4:
    Print (
     L"        Edge-triggered\n"
     );
    break;
  case 0xc:
    Print (
     L"        Level-triggered\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"    ACPI Processor UID ................................... 0x%08x\n",
    LocalX2ApicNmi->AcpiProcessorUid
    );
  Print (
    L"    Local X2APIC LINT .................................... 0x%02x\n",
    LocalX2ApicNmi->LocalX2ApicLint
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicGic (
  EFI_ACPI_5_1_GIC_STRUCTURE           *Gic
  )
{
  if (Gic == NULL) {
    return;
  }
  
  Print (         
    L"  ***************************************************************************\n"
    L"  *       Processor Local GIC Structure                                     *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Processor Local GIC address ............................ 0x%016lx\n" :
    L"  Processor Local GIC address ............................ 0x%08x\n",
    Gic
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    Gic->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    Gic->Length
    );
  if (Gic->Length >= sizeof(EFI_ACPI_5_1_GIC_STRUCTURE)) {
    Print (
      L"    CPU Interface Number ................................. 0x%08x\n",
      Gic->CPUInterfaceNumber
      );
  } else {
    Print (
      L"    GIC ID ............................................... 0x%08x\n",
      Gic->CPUInterfaceNumber
      );
  }
  Print (
    L"    ACPI Processor UID ................................... 0x%08x\n",
    Gic->AcpiProcessorUid
    );
  Print (
    L"    Flags ................................................ 0x%08x\n",
    Gic->Flags
    );
  Print (
    L"      Enabled ............................................ 0x%08x\n",
    Gic->Flags & EFI_ACPI_5_0_GIC_ENABLED
    );
  Print (
    L"      Performance Interrupt Model ........................ 0x%08x\n",
    Gic->Flags & EFI_ACPI_5_0_PERFORMANCE_INTERRUPT_MODEL
    );
  if (Gic->Length >= sizeof(EFI_ACPI_5_1_GIC_STRUCTURE)) {
    Print (
      L"      VGIC Maintenance Interrupt Model ................... 0x%08x\n",
      Gic->Flags & EFI_ACPI_5_1_VGIC_MAINTENANCE_INTERRUPT_MODE_FLAGS
      );
  }
  Print (
    L"    Parking Protocol Version ............................. 0x%08x\n",
    Gic->ParkingProtocolVersion
    );
  Print (
    L"    Performance Interrupt GSIV ........................... 0x%08x\n",
    Gic->PerformanceInterruptGsiv
    );
  Print (
    L"    Parked Address ....................................... 0x%016lx\n",
    Gic->ParkedAddress
    );
  Print (
    L"    Physical Base Address ................................ 0x%016lx\n",
    Gic->PhysicalBaseAddress
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );
  if (Gic->Length >= sizeof(EFI_ACPI_5_1_GIC_STRUCTURE)) {
    Print (
      L"    GICV ................................................. 0x%016lx\n",
      Gic->GICV
      );
    Print (
      L"    GICH ................................................. 0x%016lx\n",
      Gic->GICH
      );
    Print (
      L"    VGIC Maintenance Interrupt ........................... 0x%08x\n",
      Gic->VGICMaintenanceInterrupt
      );
    Print (
      L"    GICR Base Address .................................... 0x%016lx\n",
      Gic->GICRBaseAddress
      );
    Print (
      L"    MPIDR ................................................ 0x%016lx\n",
      Gic->MPIDR
      );
  }

  return;
}

VOID
DumpApicGicDistributor (
  EFI_ACPI_5_0_GIC_DISTRIBUTOR_STRUCTURE           *GicDistributor
  )
{
  if (GicDistributor == NULL) {
    return;
  }
  
  Print (         
    L"  ***************************************************************************\n"
    L"  *       GIC Distributor Structure                                         *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  GIC Distributor address ................................ 0x%016lx\n" :
    L"  GIC Distributor address ................................ 0x%08x\n",
    GicDistributor
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    GicDistributor->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    GicDistributor->Length
    );
  Print (
    L"    GIC ID ............................................... 0x%08x\n",
    GicDistributor->GicId
    );
  Print (
    L"    Physical Base Address ................................ 0x%016lx\n",
    GicDistributor->PhysicalBaseAddress
    );
  Print (
    L"    System Vector Base ................................... 0x%08x\n",
    GicDistributor->SystemVectorBase
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicGicMsiFrame (
  EFI_ACPI_5_1_GIC_MSI_FRAME_STRUCTURE           *GicMsiFrame
  )
{
  if (GicMsiFrame == NULL) {
    return;
  }
  
  Print (         
    L"  ***************************************************************************\n"
    L"  *       GIC MSI Frame Structure                                           *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  GIC MSI Frame address .................................. 0x%016lx\n" :
    L"  GIC MSI Frame address .................................. 0x%08x\n",
    GicMsiFrame
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    GicMsiFrame->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    GicMsiFrame->Length
    );
  Print (
    L"    GIC MSI Frame ID ..................................... 0x%08x\n",
    GicMsiFrame->GicMsiFrameId
    );
  Print (
    L"    Physical Base Address ................................ 0x%016lx\n",
    GicMsiFrame->PhysicalBaseAddress
    );
  Print (
    L"    Flags ................................................ 0x%08x\n",
    GicMsiFrame->Flags
    );
  Print (
    L"      SPI Count Base Select .............................. 0x%08x\n",
    GicMsiFrame->Flags & EFI_ACPI_5_1_SPI_COUNT_BASE_SELECT
    );
  Print (
    L"    SPI Count ............................................ 0x%04x\n",
    GicMsiFrame->SPICount
    );
  Print (
    L"    SPI Base ............................................. 0x%04x\n",
    GicMsiFrame->SPIBase
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpApicGicRedistributor (
  EFI_ACPI_5_1_GICR_STRUCTURE           *GicRedistributor
  )
{
  if (GicRedistributor == NULL) {
    return;
  }
  
  Print (         
    L"  ***************************************************************************\n"
    L"  *       GIC Redistributor Structure                                       *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  GIC Redistributor address .............................. 0x%016lx\n" :
    L"  GIC Redistributor address .............................. 0x%08x\n",
    GicRedistributor
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    GicRedistributor->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    GicRedistributor->Length
    );
  Print (
    L"    Discovery Range Base Address ......................... 0x%016lx\n",
    GicRedistributor->DiscoveryRangeBaseAddress
    );
  Print (
    L"    Discovery Range Length ............................... 0x%08x\n",
    GicRedistributor->DiscoveryRangeLength
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiMADT (
  VOID  *Table
  )
{
  EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER                            *Madt;
  APIC_STRUCT_HEADER        *ApicStructHeader;
  INTN                      MadtLen;
  INTN                      TableLen;

  Madt = Table;
  if (Madt == NULL) {
    return;
  }
  
  //
  // Dump Madt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Multiple APIC Description Table                                   *\n"
    L"*****************************************************************************\n"
    );

  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Madt->Header.Length, Madt);
  }

  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (         
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"MADT address ............................................. 0x%016lx\n" :
    L"MADT address ............................................. 0x%08x\n",
    Madt
    );
  
  DumpAcpiTableHeader (&(Madt->Header));
  
  Print (         
    L"  Table Contents:\n"
    );
  Print (         
    L"    Local APIC Address ................................... 0x%02x\n",
    Madt->LocalApicAddress
    );
  Print (         
    L"    Flags ................................................ 0x%02x\n",
    Madt->Flags
    );
  Print (
    L"      PCAT_COMPAT ........................................ 0x%02x\n",
    Madt->Flags & EFI_ACPI_1_0_PCAT_COMPAT
    );
  
  Print (
    L"\n"
    );
  
  MadtLen  = Madt->Header.Length - sizeof(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);
  TableLen = 0;
  ApicStructHeader = (APIC_STRUCT_HEADER *)(Madt + 1);
  while (MadtLen > 0) {
    switch (ApicStructHeader->Type) {
    case EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC:
      DumpApicProcessorLocalApic ((EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC_STRUCTURE);
      break;
    case EFI_ACPI_1_0_IO_APIC:
      DumpApicIOApic ((EFI_ACPI_1_0_IO_APIC_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_1_0_IO_APIC_STRUCTURE);
      break;
    case EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE:
      DumpApicInterruptSourceOverride ((EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE);
      break;
    case EFI_ACPI_1_0_NON_MASKABLE_INTERRUPT_SOURCE:
      DumpApicNonMaskableInterruptSource ((EFI_ACPI_1_0_NON_MASKABLE_INTERRUPT_SOURCE_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_1_0_NON_MASKABLE_INTERRUPT_SOURCE_STRUCTURE);
      break;
    case EFI_ACPI_1_0_LOCAL_APIC_NMI:
      DumpApicLocalApicNMI ((EFI_ACPI_1_0_LOCAL_APIC_NMI_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_1_0_LOCAL_APIC_NMI_STRUCTURE);
      break;
    case EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE:
      DumpApicLocalApicAddressOverride ((EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE_STRUCTURE);
      break;
    case EFI_ACPI_2_0_IO_SAPIC:
      DumpApicIOSapic ((EFI_ACPI_2_0_IO_SAPIC_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_2_0_IO_SAPIC_STRUCTURE);
      break;
    case EFI_ACPI_2_0_PROCESSOR_LOCAL_SAPIC:
      DumpApicProcessorLocalSapic ((EFI_ACPI_2_0_PROCESSOR_LOCAL_SAPIC_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_2_0_PROCESSOR_LOCAL_SAPIC_STRUCTURE);
      break;
    case EFI_ACPI_2_0_PLATFORM_INTERRUPT_SOURCES:
      DumpApicPlatformInterruptSource ((EFI_ACPI_2_0_PLATFORM_INTERRUPT_SOURCES_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_2_0_PLATFORM_INTERRUPT_SOURCES_STRUCTURE);
      break;
    case EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC:
      DumpApicProcessorLocalX2Apic ((EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE);
      break;
    case EFI_ACPI_4_0_LOCAL_X2APIC_NMI:
      DumpApicLocalX2ApicNmi ((EFI_ACPI_4_0_LOCAL_X2APIC_NMI_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_4_0_LOCAL_X2APIC_NMI_STRUCTURE);
      break;
    case EFI_ACPI_5_0_GIC:
      DumpApicGic ((EFI_ACPI_5_1_GIC_STRUCTURE *)ApicStructHeader);
      TableLen = ApicStructHeader->Length;
      break;
    case EFI_ACPI_5_0_GICD:
      DumpApicGicDistributor ((EFI_ACPI_5_0_GIC_DISTRIBUTOR_STRUCTURE *)ApicStructHeader);
      TableLen = ApicStructHeader->Length;
      break;
    case EFI_ACPI_5_1_GIC_MSI_FRAME:
      DumpApicGicMsiFrame ((EFI_ACPI_5_1_GIC_MSI_FRAME_STRUCTURE *)ApicStructHeader);
      TableLen = ApicStructHeader->Length;
      break;
    case EFI_ACPI_5_1_GICR:
      DumpApicGicRedistributor ((EFI_ACPI_5_1_GICR_STRUCTURE *)ApicStructHeader);
      TableLen = ApicStructHeader->Length;
      break;
    default:
      break;
    }
    ApicStructHeader = (APIC_STRUCT_HEADER *)((UINT8 *)ApicStructHeader + ApicStructHeader->Length);
    MadtLen         -= TableLen;
  }

Done:
  Print (
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiMADTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_1_0_APIC_SIGNATURE, DumpAcpiMADT);
}
