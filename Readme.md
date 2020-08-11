# This package includes some debug tool for EDKII BIOS.

## How to build?
Just build it as a normal EDKII package.

  0) Download EDKII from github https://github.com/tianocore/edk2

  1) Put EdkiiShellToolPkg to root directory

  2) Build EdkiiShellToolPkg as normal way.

## Feature:
- Gcd

A tool to dump GCD data structure, according to PI specification. 

- HobList

A tool to dump HOB data structure, according to PI specification. 

- MemoryAttributesDump

A tool to dump Memory Attribute Table, according to UEFI specification. 

- HstiWsmtDump

A tool to dump HSTI table and WSMT table, according to Microsoft HSTI and WSMT specification. 

- EsrtFmpDump

A tool to dump ESRT table and FMP capsule information, according to UEFI specification. 

- MemoryTypeInfo

A tool to dump EDKII memory type information, according to EDKII implementation. 

- PerfDump

A tool to dump EDKII performance data, according to EDKII implementation. 

- PcdDump

A tool to dump PCD information according to PI specification and PCD internal database according to EDKII implementation. 

- SmmProfileDump

A tool to dump EDKII SMM profile data, according to EDKII implementation. 

- EdkiiCoreDatabaseDump

Tools to dump EDKII DXE Core, SMM Core, and PEI Core internal data structure, according to EDKII implementation.

A user may run DxeCoreDumpApp in SHELL directly.
A user need include PeiCoreDump.inf in BIOS to run PeiCoreDumpApp in UEFI SHELL.
A user need include SmmCoreDump.inf in BIOS to run SmmCoreDumpApp in UEFI SHELL.

These core database dump features are only for debug purpose. Please do not include these PeiCoreDump and SmmCoreDump in a production BIOS.

- GetPciOprom

A tool to dump PCI OPROM information and save PCI OPROM to file.

- PatchMicrocode

A tool to patch the CPU Microcode.

- VtdDump

A tool to dump VTD information according to VTd specification and IOMMU map information according to EDKII implementation. 

- Tcg2Dump

A tool to dump TCG2 event log, according to TCG specification. 

- StackUage

A tool to check the stack usage in an EFI application.
This tool shows the stack size, current stack location and the stack touched by a UEFI application. 
UEFI specification requires 128K as minimal. We observed that some special programs require more.
This tool helps a developer to adjust the stack size.

- DbxEnroll

A tool to enroll the [revocation list](https://uefi.org/revocationlistfile) in UEFI shell.

## Known limitation:
This package is only the sample code to show the concept.
It does not have a full validation and does not meet the production quality yet.

Some tools use edk2 open source package internal data structure. They work only on a UEFI BIOS based on open source edk2 package. 
