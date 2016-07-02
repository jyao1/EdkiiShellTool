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

A tool to dump Memory Attribute Table and Properties Table, according to UEFI specification. 

- HstiWsmtDump

A tool to dump HSTI table and WSMT table, according to Microsoft HSTI and WSMT specification. 

- EsrtFmpDump

A tool to dump ESRT table and FMP capsule information, according to UEFI specification. 

- MemoryTypeInfo

A tool to dump EDKII memory type information, according to EDKII implementation. 

- PerfDump

A tool to dump EDKII performance data, according to EDKII implementation. 

- SmmProfileDump

A tool to dump EDKII SMM profile data, according to EDKII implementation. 

- EdkiiCoreDatabaseDump

Tools to dump EDKII DXE Core, SMM Core, and PEI Core internal data structure, according to EDKII implementation.

## Known limitation:
Some tools use edk2 open source package internal data structure. They work only on a UEFI BIOS based on open source edk2 package. 
