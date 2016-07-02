/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PerformanceLib.h>
#include <Protocol/LoadedImage.h>

#include <Guid/Performance.h>

#define PERFORMANCE_HANDLE_NAME_SIZE  128

typedef struct {
  CONST VOID            *Handle;
  CONST CHAR8           *Token;           ///< Measured token string name.
  CONST CHAR8           *Module;          ///< Module string name.
  UINT64                StartTimeStamp;   ///< Start time point.
  UINT64                EndTimeStamp;     ///< End time point.
  UINT32                Identifier;       ///< Identifier.
} MEASUREMENT_RECORD;

/**
  Get the short verion of PDB file name to be
  used in performance data logging.

  @param PdbFileName     The long PDB file name.
  @param GaugeString     The output string to be logged by performance logger.
  @param StringSize      The buffer size of GaugeString in bytes.

**/
VOID
GetShortPdbFileName (
  IN  CONST CHAR8  *PdbFileName,
  OUT       CHAR8  *GaugeString,
  IN        UINTN   StringSize
  )
{
  UINTN Index;
  UINTN Index1;
  UINTN StartIndex;
  UINTN EndIndex;

  if (PdbFileName == NULL) {
    AsciiStrCpyS (GaugeString, StringSize, " ");
  } else {
    StartIndex = 0;
    for (EndIndex = 0; PdbFileName[EndIndex] != 0; EndIndex++)
      ;

    for (Index = 0; PdbFileName[Index] != 0; Index++) {
      if (PdbFileName[Index] == '\\') {
        StartIndex = Index + 1;
      }

      if (PdbFileName[Index] == '.') {
        EndIndex = Index;
      }
    }

    Index1 = 0;
    for (Index = StartIndex; Index < EndIndex; Index++) {
      GaugeString[Index1] = PdbFileName[Index];
      Index1++;
      if (Index1 == StringSize - 1) {
        break;
      }
    }

    GaugeString[Index1] = 0;
  }

  return ;
}

/**
  Get the name from the Driver handle, which can be a handle with
  EFI_LOADED_IMAGE_PROTOCOL or EFI_DRIVER_BINDING_PROTOCOL installed.
  This name can be used in performance data logging.

  @param Handle          Driver handle.
  @param GaugeString     The output string to be logged by performance logger.
  @param StringSize      The buffer size of GaugeString in bytes.

**/
VOID
GetNameFromHandle (
  IN  EFI_HANDLE     Handle,
  OUT CHAR8          *GaugeString,
  IN  UINTN          StringSize
  )
{
  EFI_STATUS                  Status;
  EFI_LOADED_IMAGE_PROTOCOL   *Image;
  CHAR8                       *PdbFileName;
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding;

  AsciiStrCpyS (GaugeString, StringSize, " ");

  //
  // Get handle name from image protocol
  //
  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **) &Image
                  );

  if (EFI_ERROR (Status)) {
    Status = gBS->OpenProtocol (
                    Handle,
                    &gEfiDriverBindingProtocolGuid,
                    (VOID **) &DriverBinding,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      return ;
    }
    //
    // Get handle name from image protocol
    //
    Status = gBS->HandleProtocol (
                    DriverBinding->ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **) &Image
                    );
  }

  PdbFileName = PeCoffLoaderGetPdbPointer (Image->ImageBase);

  if (PdbFileName != NULL) {
    GetShortPdbFileName (PdbFileName, GaugeString, StringSize);
  }

  return ;
}

BOOLEAN
GetCountUp(
  VOID
  )
{
  UINTN                     LogEntryKey;
  MEASUREMENT_RECORD        Measurement;
  UINTN                     Up;
  UINTN                     Down;

  LogEntryKey = 0;
  Up = 0;
  Down = 0;
  while ((LogEntryKey = GetPerformanceMeasurementEx (
                        LogEntryKey,
                        &Measurement.Handle,
                        &Measurement.Token,
                        &Measurement.Module,
                        &Measurement.StartTimeStamp,
                        &Measurement.EndTimeStamp,
                        &Measurement.Identifier)) != 0) {
    if (Measurement.StartTimeStamp == 1) {
      continue;
    }

    if (Measurement.StartTimeStamp <= Measurement.EndTimeStamp) {
      Up++;
    } else {
      Down++;
    }
  }

  if (Up / 4 >= Down) {
    return TRUE;
  }
  if (Down / 4 >= Up) {
    return FALSE;
  }
  if (Up >= Down) {
    Print(L"WARNIN: Assume TimeStamp CountUp\n");
    return TRUE;
  } else {
    Print(L"WARNIN: Assume TimeStamp CountDown\n");
    return FALSE;
  }
}

UINT64
GetFreqency(
  VOID
  )
{
  EFI_STATUS              Status;
  UINTN                   Size;
  EFI_PHYSICAL_ADDRESS    PerfDataMemAddr;

  Size = sizeof(PerfDataMemAddr);
  Status = gRT->GetVariable (
                  L"PerfDataMemAddr",
                  &gPerformanceProtocolGuid,
                  NULL,
                  &Size,
                  &PerfDataMemAddr
                  );
  if (!EFI_ERROR(Status)) {
    return ((PERF_HEADER *)(UINTN)PerfDataMemAddr)->CpuFreq;
  }
  return 0;
}

VOID
DumpGauge(
  VOID
  )
{
  UINTN                     LogEntryKey;
  MEASUREMENT_RECORD        Measurement;
  CHAR8                     GaugeString[PERFORMANCE_HANDLE_NAME_SIZE];
  EFI_HANDLE                *Handles;
  UINTN                     NoHandles;
  UINTN                     Index;
  EFI_STATUS                Status;
  BOOLEAN                   TotalTime;
  BOOLEAN                   CountUp;
  UINT64                    Tick;
  UINT64                    Duration;
  UINT64                    Freq;

  CountUp = GetCountUp();
  Freq = GetFreqency();
  Duration = 0;

  Handles = NULL;
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &NoHandles,
                  &Handles
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }

  LogEntryKey = 0;
  Print(L"   Handle     Token    Module     Start     End     Identifier\n");
  Print(L"  ========  ========  ========  ========  ========  ========\n");
  while ((LogEntryKey = GetPerformanceMeasurementEx (
                        LogEntryKey,
                        &Measurement.Handle,
                        &Measurement.Token,
                        &Measurement.Module,
                        &Measurement.StartTimeStamp,
                        &Measurement.EndTimeStamp,
                        &Measurement.Identifier)) != 0) {
    if ((Measurement.StartTimeStamp == 1) || (Measurement.EndTimeStamp == 1)) {
      TotalTime = TRUE;
    } else {
      TotalTime = FALSE;
    }
    Print(
      L"  %08x  [%a]  [%a]  %ld  %ld  %08x",
      Measurement.Handle,
      Measurement.Token,
      Measurement.Module,
      Measurement.StartTimeStamp,
      Measurement.EndTimeStamp,
      Measurement.Identifier
      );

    if (Measurement.StartTimeStamp == 1) {
      Tick = Measurement.EndTimeStamp;
    } if (Measurement.EndTimeStamp == 1) {
      Tick = Measurement.StartTimeStamp;
    } else {
      if (CountUp) {
        Tick = Measurement.EndTimeStamp - Measurement.StartTimeStamp;
      } else {
        Tick = Measurement.StartTimeStamp - Measurement.EndTimeStamp;
      }
    }
    if (Freq != 0) {
      Duration = DivU64x32(Tick, (UINT32)Freq);
    }
    if (Duration != 0) {
      Print(L"  {%ld}", Duration);
    }

    for (Index = 0; Index < NoHandles; Index++) {
      if (Measurement.Handle == Handles[Index]) {
        ZeroMem(GaugeString, sizeof(GaugeString));
        GetNameFromHandle((EFI_HANDLE)Measurement.Handle, GaugeString, PERFORMANCE_HANDLE_NAME_SIZE);
        if (GaugeString[0] != 0) {
          Print(L"  (%a)", GaugeString);
        }
        break;
      }
    }
    if (TotalTime) {
      Print(L"  (*)");
    }
    Print(L"\n");
  }
  Print(L"  ========  ========  ========  ========  ========  ========\n");
  Print(L"(*) means total time from timer start\n");

  FreePool(Handles);
}

VOID
DumpPerfHeader (
  IN PERF_HEADER      *PerfHeader
  )
{
  Print(L"  BootToOs    - 0x%016lx\n", PerfHeader->BootToOs);
  Print(L"  S3Resume    - 0x%016lx\n", PerfHeader->S3Resume);
  Print(L"  S3EntryNum  - 0x%08x\n", PerfHeader->S3EntryNum);
  Print(L"  CpuFreq     - 0x%016lx\n", PerfHeader->CpuFreq);
  Print(L"  BDSRaw      - 0x%016lx\n", PerfHeader->BDSRaw);
  Print(L"  Count       - 0x%08x\n", PerfHeader->Count);
  Print(L"  Signiture   - 0x%08x\n", PerfHeader->Signiture);
}

VOID
DumpPerfData (
  IN PERF_DATA      *PerfData
  )
{
  Print(L"  [%a] - %d\n", PerfData->Token, PerfData->Duration);
}

VOID
DumpPerf (
  IN PERF_HEADER      *PerfHeader
  )
{
  PERF_DATA      *PerfData;
  UINTN          Index;

  Print(L"PERF_HEADER\n");
  DumpPerfHeader(PerfHeader);
  PerfData = (PERF_DATA *)(UINTN)(PerfHeader + 1);

  Print(L"PERF_DATA\n");
  for (Index = 0; Index < (UINTN)PerfHeader->Count; Index++) {
    DumpPerfData(&PerfData[Index]);
  }
  Print(L"S3Entry\n");
  for (Index = 0; Index < (UINTN)PerfHeader->S3EntryNum; Index++) {
    DumpPerfData(&PerfHeader->S3Entry[Index]);
  }
  return;
}

EFI_STATUS
EFIAPI
PerfDumpEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;
  UINTN                   Size;
  EFI_PHYSICAL_ADDRESS    PerfDataMemAddr;

  Print(L"##############\n");
  Print(L"# GAUGE DATA #\n");
  Print(L"##############\n");
  DumpGauge();

  Print(L"#############\n");
  Print(L"# PERF DATA #\n");
  Print(L"#############\n");
  Size = sizeof(PerfDataMemAddr);
  Status = gRT->GetVariable (
                  L"PerfDataMemAddr",
                  &gPerformanceProtocolGuid,
                  NULL,
                  &Size,
                  &PerfDataMemAddr
                  );
  if (!EFI_ERROR(Status)) {
    DumpPerf ((PERF_HEADER *)(UINTN)PerfDataMemAddr);
  } else {
    Print (L"PerfDataMemAddr not found!\n");
  }

  return EFI_SUCCESS;
}
