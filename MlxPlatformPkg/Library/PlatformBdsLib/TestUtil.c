#include "TestUtil.h"

#include <Guid/GlobalVariable.h>

#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SerialPortLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

//
// Note we use different names for volatile and non-volatile
// variables, so that we don't move variables from one store to the
// other, which is not obviously supported.
//
// Note that all these names must have the same length as some
// functions require that (e.g. TestAppendWriteVariables).
//
CHAR16 *
VolatileVarNames[] = {
  L"TestVar0", L"TestVar1", L"TestVar2", L"TestVar3", L"TestVar4",
  L"TestVar5", L"TestVar6", L"TestVar7", L"TestVar8", L"TestVar9"
};

//
// Note that all these names must have the same length as some
// functions require that (e.g. TestAppendWriteVariables).
//
CHAR16 *
NonVolatileVarNames[] = {
  L"TestNVar0", L"TestNVar1", L"TestNVar2", L"TestNVar3", L"TestNVar4",
  L"TestNVar5", L"TestNVar6", L"TestNVar7", L"TestNVar8", L"TestNVar9"
};

//
// We want the number of variables and the number of strings to be
// relatively prime, to maximize variations of strings written between
// iterations.  Include strings of length 0, 1, and length greater
// than block size (512).
//
CHAR16 *
RandomStrings[] = {
  L"0afdoifsadofjisadofisdafiadoaoisdffjiodofjiojioffoadfjiadofisfdj"
  L"ijsadofiasdfjoiwaerhnawera,werapovsdpfvomk;zdw3904230432130kaemk"
  L"irjwojrmkwelrealfv/sdafs,.,.asf942398klafs';wae0r;alfwekfaklsdff"
  L"fjisoarwelka.li23o4ojmkladw.waei9cio jmofwe9kfdaodkopfasdpofsadp"
  L"afsdiojfsados89deuerjiowairwe0we9r89i23kiaefkliofjsdiofsadfisado",
  L"",
  L"2",
  L"3@",
  L"4iujxzswojik",
  L"5afjdiojoifd@",
  L"6xei2qohjnka"
};

//
// Define the maximum supported error message length.
//
#define MAX_ERROR_LENGTH  0x100

/**
  Print out an error message to the serial port, and hang.

  @param  Format      Format string of the error message.
  @param  ...         Variable argument list whose contents are accessed
                      based on the format string specified by Format.

**/
VOID
ReportError (
  IN  CONST CHAR8  *Format,
  ...
  )
{
  CHAR8   *ErrorPrefix = "UEFI error: ";
  CHAR8    Buffer[MAX_ERROR_LENGTH];
  VA_LIST  Marker;

  VA_START (Marker, Format);
  AsciiVSPrint (Buffer, sizeof (Buffer), Format, Marker);
  VA_END (Marker);

  SerialPortWrite ((UINT8 *)ErrorPrefix, AsciiStrLen (ErrorPrefix));
  SerialPortWrite ((UINT8 *)Buffer, AsciiStrLen (Buffer));
  CpuDeadLoop ();
}

/**
  Return the number of test volatile variables.

  @return The number of test volatile variables.

**/
UINTN
NumVolatileVariables (
  )
{
  return sizeof (VolatileVarNames) / sizeof (*VolatileVarNames);
}

/**
  Return the number of test non-volatile variables.

  @return The number of test non-volatile variables.

**/
UINTN
NumNonVolatileVariables (
  )
{
  return sizeof (NonVolatileVarNames) / sizeof (*NonVolatileVarNames);
}

/**
  Return the number of test random strings.

  @return The number of test random strings.

**/
UINTN
NumRandomStrings (
  )
{
  return sizeof (RandomStrings) / sizeof (*RandomStrings);
}

/**
  Return the size of the largest string in RandomStrings.

  @return The size of the largest string in RandomStrings.

**/
UINTN
GetMaxRandomStringSize (
  )
{
  UINTN  NumStrings;
  UINTN  Size;
  UINTN  I;

  NumStrings = NumRandomStrings ();

  Size = 0;
  for (I = 0; I < NumStrings; I++) {
    Size = MAX (Size, StrSize (RandomStrings[I]));
  }

  return Size;
}

/**
  Write all the variables, using the RandomStrings round robin as the
  values of the variables, starting at the input StringIndex.

  @param  StringIndex             The next index to use for the random strings.
  @param  VariableAttributes      The attributes to use for variables (e.g. volatile, etc.).
  @param  VarNames                Array of variable names.
  @param  NumVariabes             Number of variables.
  @param  RandomStrings           Array of strings to use for variable values.
  @param  NumStrings              Number of strings.
  @param  ValueIndex              If not NULL, records the indices of the RandomStrings that
                                  are written to each variable.

  @return Number of write errors.

**/
UINTN
WriteAllVariables (
  IN OUT UINTN     *StringIndex,
  IN UINT32         VariableAttributes,
  IN CHAR16       **VarNames,
  IN UINTN          NumVariables,
  IN CHAR16       **RandomStrings,
  IN UINTN          NumStrings,
  OUT UINTN        *ValueIndex
  )
{
  EFI_STATUS Status;
  CHAR16    *StringValue;
  UINTN      I;
  UINTN      Errors;

  Errors = 0;
  for (I = 0; I < NumVariables; I++) {
    StringValue = RandomStrings[*StringIndex];
    if (ValueIndex != NULL) {
      ValueIndex[I] = *StringIndex;
    }
    *StringIndex = (*StringIndex + 1) % NumStrings;
    Status = gRT->SetVariable (VarNames[I], &gEfiGlobalVariableGuid, VariableAttributes,
                               StrSize (StringValue), StringValue);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "variable %s write failed: %r\n", VarNames[I], Status));
      Errors++;
    }
  }

  return Errors;
}

/**
  Read all the variables and check their values against what is
  expected.

  @param  VariableAttributes      The expected attributes of the variables.
  @param  VarNames                Array of variable names.
  @param  NumVariabes             Number of variables.
  @param  RandomStrings           Array of strings values.
  @param  NumStrings              Number of strings.
  @param  ValueIndex              Expected value of each variable, represented by
                                  indices into the RandomStrings array.

  @return Number of errors encountered.

**/
UINTN
ReadVerifyAllVariables (
  IN UINT32         VariableAttributes,
  IN CHAR16       **VarNames,
  IN UINTN          NumVariables,
  IN CHAR16       **RandomStrings,
  IN UINTN          NumStrings,
  IN UINTN         *ValueIndex
  )
{
  UINTN      MaxStringSize;
  EFI_STATUS Status;
  UINT32     Attributes;
  UINTN      DataSize;
  CHAR16    *DataValue;
  UINTN      I;
  UINTN      Errors;

  MaxStringSize = GetMaxRandomStringSize ();

  DataValue = AllocatePool (MaxStringSize);
  ASSERT (DataValue != NULL);

  Errors = 0;
  for (I = 0; I < NumVariables; I++) {
    DataSize = MaxStringSize;
    Status = gRT->GetVariable (VarNames[I], &gEfiGlobalVariableGuid, &Attributes,
                               &DataSize, DataValue);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "variable %s read failed: %r\n", VarNames[I], Status));
      Errors++;
      continue;
    }

    if (Attributes != VariableAttributes) {
      DEBUG ((EFI_D_ERROR, "variable %s wrong attributes expect 0x%x got 0x%x\n",
              VarNames[I], VariableAttributes, Attributes));
      Errors++;
    }

    if (StrCmp (RandomStrings[ValueIndex[I]], DataValue) != 0) {
      DEBUG ((EFI_D_ERROR, "variable %s wrong value expect %s got %s\n",
              VarNames[I], RandomStrings[ValueIndex[I]], DataValue));
      Errors++;
    }
  }

  FreePool (DataValue);

  return Errors;
}

/**
  Verify that the value of a variable matches the expected value.

  @param  Name           Name of the variable.
  @param  ExpectedValue  Expected value of the variable.

**/
UINTN
ReadVerifyVariable (
  IN CHAR16       *Name,
  IN CHAR16       *ExpectedValue
  )
{
  EFI_STATUS Status;
  CHAR16   **VarNames;
  UINTN      DataSize;
  CHAR16    *DataValue;
  UINTN      Errors;

  VarNames = NonVolatileVarNames;

  DataSize = GetMaxRandomStringSize ();
  DataValue = AllocatePool (DataSize);
  ASSERT (DataValue != NULL);

  Status = gRT->GetVariable (Name, &gEfiGlobalVariableGuid, NULL,
                             &DataSize, DataValue);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "variable %s read error: %r\n", VarNames[4], Status));
    return 1;
  }

  Errors = 0;
  if (StrCmp (DataValue, ExpectedValue) != 0) {
    DEBUG ((EFI_D_ERROR, "variable %s wrong value  expect %s got %s\n",
            VarNames[4], ExpectedValue, DataValue));
    Errors++;
  }

  FreePool (DataValue);

  return Errors;
}
