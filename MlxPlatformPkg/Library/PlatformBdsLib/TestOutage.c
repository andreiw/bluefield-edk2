#include "TestUtil.h"

#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Variable.h>

#include <Guid/GlobalVariable.h>

/**
  Simulate a power outage in the middle of an NV variable write.  This
  is done by setting some bits in the attributes passed to SetVariable
  reserved for this hack.

  @param  Type        The outage type.
**/
STATIC
UINTN
SimulateWriteOutage (
  UINTN  Type
  )
{
  EFI_STATUS Status;
  CHAR16   **VarNames;
  UINTN      NumVariables;
  UINTN      NumStrings;
  UINTN      MaxStringSize;
  CHAR16    *DataValue;
  UINT32     Attributes;
  UINTN      StringIndex;
  UINTN      Errors;

  VarNames = NonVolatileVarNames;

  NumVariables = NumNonVolatileVariables ();
  NumStrings = NumRandomStrings ();

  MaxStringSize = GetMaxRandomStringSize ();

  DataValue = AllocatePool (MaxStringSize);
  ASSERT (DataValue != NULL);

  Attributes = (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS |
                EFI_VARIABLE_NON_VOLATILE);

  StringIndex = 0;
  Errors = WriteAllVariables (&StringIndex, Attributes, VarNames, NumVariables,
                              RandomStrings, NumStrings, NULL);

  Status = gRT->SetVariable (VarNames[4], &gEfiGlobalVariableGuid,
                             Attributes | (Type << EFI_VARIABLE_OUTAGES_SHIFT),
                             StrSize (L"OutageWrite"), L"OutageWrite");
  if (EFI_ERROR (Status)) {
    Errors++;
  }

  FreePool (DataValue);

  return Errors;
}

/**
  First half of a test simulating a power outage in the middle of a
  persistent variable write.

**/
VOID
TestOutageWrite1 (
  )
{
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestOutageWrite1\n"));
  Errors = SimulateWriteOutage (1);
  if (Errors != 0) {
    ReportError ("TestOutageWrite1: %d errors.\n", Errors);
  }
}

/**
  First half of a test simulating a power outage in the middle of a
  persistent variable write.

**/
VOID
TestOutageWrite2 (
  )
{
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestOutageWrite2\n"));
  Errors = SimulateWriteOutage (2);
  if (Errors != 0) {
    ReportError ("TestOutageWrite2: %d errors.\n", Errors);
  }
}

/**
  First half of a test simulating a power outage in the middle of a
  persistent variable write.

**/
VOID
TestOutageWrite3 (
  )
{
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestOutageWrite3\n"));
  Errors = SimulateWriteOutage (3);
  if (Errors != 0) {
    ReportError ("TestOutageWrite3: %d errors.\n", Errors);
  }
}

/**
  Second half of a test simulating outage in the middle of an NVStore
  write.  Verify that the value of a variable after the simulated
  outage is as expected.

**/
VOID
TestOutageRead1 (
  )
{
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestOutageRead1\n"));

  Errors = ReadVerifyVariable (NonVolatileVarNames[4], RandomStrings[4]);
  if (Errors != 0) {
    ReportError ("TestOutageRead1: %d errors.\n", Errors);
  }
}

/**
  Second half of a test simulating outage in the middle of an NVStore
  write.  Verify that the value of a variable after the simulated
  outage is as expected.

**/
VOID
TestOutageRead2 (
  )
{
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestOutageRead2\n"));

  Errors = ReadVerifyVariable (NonVolatileVarNames[4], RandomStrings[4]);
  if (Errors != 0) {
    ReportError ("TestOutageRead2: %d errors.\n", Errors);
  }
}

/**
  Second half of a test simulating outage in the middle of an NVStore
  write.  Verify that the value of a variable after the simulated
  outage is as expected.

**/
VOID
TestOutageRead3 (
  )
{
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestOutageRead3\n"));

  Errors = ReadVerifyVariable (NonVolatileVarNames[4], L"OutageWrite");
  if (Errors != 0) {
    ReportError ("TestOutageRead3: %d errors.\n", Errors);
  }
}
