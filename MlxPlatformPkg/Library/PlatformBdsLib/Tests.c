/** @file
*
*  Copyright (c) 2016, Mellanox Technologies. All rights reserved.
*
*  This program and the accompanying materials are licensed and made
*  available under the terms and conditions of the BSD License which
*  accompanies this distribution.  The full text of the license may be
*  found at http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
*  BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER
*  EXPRESS OR IMPLIED.
*
**/

#include "BdsPlatform.h"
#include "TestOutage.h"
#include "TestUtil.h"

#include <Guid/VariableFormat.h>

/**
  Test basic variable write and read.  Write all the variables, then
  read them and check their values.  Use the RandomStrings round robin
  as the values of the variables, starting at the input StringIndex.

  @param  StringIndex             The next index to use for the random strings.
  @param  VariableAttributes      The attributes to use for variables (e.g. volatile, etc.).
  @param  VarNames                Array of variable names.
  @param  NumVariabes             Number of variables.
  @param  RandomStrings           Array of strings to use for variable values.
  @param  NumStrings              Number of strings.

  @return Number of errors encountered.

**/
STATIC
UINTN
TestVariableWriteRead (
  IN OUT UINTN     *StringIndex,
  IN UINT32         VariableAttributes,
  IN CHAR16       **VarNames,
  IN UINTN          NumVariables,
  IN CHAR16       **RandomStrings,
  IN UINTN          NumStrings
  )
{
  UINTN      ValueIndex[NumVariables];
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestVariableWriteRead\n"));

  Errors = 0;
  Errors += WriteAllVariables (StringIndex, VariableAttributes, VarNames, NumVariables,
                               RandomStrings, NumStrings, ValueIndex);
  Errors += ReadVerifyAllVariables (VariableAttributes, VarNames, NumVariables,
                                    RandomStrings, NumStrings, ValueIndex);

  return Errors;
}

/**
  Test variable reclaim function.  Write enough variables to force a
  reclaim, and verify that the variables retain the expected values
  afterwards.

  @param  VariableAttributes      The attributes to use for variables (e.g. volatile, etc.).
  @param  VarNames                Array of variable names.
  @param  NumVariabes             Number of variables.
  @param  RandomStrings           Array of strings to use for variable values.
  @param  NumStrings              Number of strings.

  @return Number of variables with erroneous values.

**/
STATIC
UINTN
TestVariableReclaim (
  IN UINT32     VariableAttributes,
  IN CHAR16   **VarNames,
  IN UINTN      NumVariables,
  IN CHAR16   **RandomStrings,
  IN UINTN      NumStrings
  )
{
  UINTN      StringTotalSize;
  UINTN      ValueIndex[NumVariables];
  UINTN      Iterations;
  UINTN      I;
  UINTN      StringIndex;
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestVariableReclaim\n"));

  // Conservatively calculate the number of iterations it takes to
  // trigger a reclaim.
  StringTotalSize = 0;
  for (I = 0; I < NumStrings; I++)
  {
    StringTotalSize += StrSize (RandomStrings[I]);
  }

  Iterations = 1 + (NumStrings * PcdGet32 (PcdVariableStoreSize) /
                    (StringTotalSize * NumVariables));

  StringIndex = 0;
  Errors = 0;
  for (I = 0; I < Iterations; I++) {
    Errors += WriteAllVariables (&StringIndex, VariableAttributes, VarNames, NumVariables,
                                 RandomStrings, NumStrings, ValueIndex);
  }

  // Read back the variables and check they have the expected values.
  Errors += ReadVerifyAllVariables (VariableAttributes, VarNames, NumVariables,
                                    RandomStrings, NumStrings, ValueIndex);

  return Errors;
}

/**
  The function runs some unit tests for variable storage.

  @param  Attributes      The attributes to use for variables (e.g. volatile, etc.).

**/
STATIC
VOID
TestVariables (
  IN UINT32     Attributes
  )
{
  CHAR16  **VarNames;
  UINTN     NumVariables;
  UINTN     NumStrings;
  UINTN     StringIndex;
  UINTN     Errors;

  if (Attributes & EFI_VARIABLE_NON_VOLATILE) {
    VarNames = NonVolatileVarNames;
    NumVariables = NumNonVolatileVariables ();
  } else {
    VarNames = VolatileVarNames;
    NumVariables = NumVolatileVariables ();
  }

  NumStrings = NumRandomStrings ();

  // Write each variable, and read back and verify the contents.
  Errors = 0;
  StringIndex = 0;
  Errors += TestVariableWriteRead (&StringIndex, Attributes, VarNames, NumVariables,
                                   RandomStrings, NumStrings);

  // Overwrite each variable, and read back and verify the contents.
  Errors += TestVariableWriteRead (&StringIndex, Attributes, VarNames, NumVariables,
                                   RandomStrings, NumStrings);

  // Ditto.
  Errors += TestVariableWriteRead (&StringIndex, Attributes, VarNames, NumVariables,
                                   RandomStrings, NumStrings);

  // Test reclaim functionality.
  Errors += TestVariableReclaim (Attributes, VarNames, NumVariables,
                                 RandomStrings, NumStrings);

  if (Errors != 0) {
    ReportError ("TestVariables: %d errors.\n", Errors);
  }
}

/**
  Basic variable store unit tests.

**/
STATIC
VOID
Testv1 (
  )
{
  DEBUG ((EFI_D_INFO, "Variable test 1\n"));

  DEBUG ((EFI_D_INFO, "Test non-volatile variables\n"));
  TestVariables (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS |
                 EFI_VARIABLE_NON_VOLATILE);

  DEBUG ((EFI_D_INFO, "Test volatile variables\n"));
  TestVariables (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS);
}

/**
  First half of a NV variable store test.  Write a bunch of variables
  to NV storage.

**/
STATIC
VOID
TestPersistentStoreWrite (
  )
{
  UINTN      NumVariables;
  UINTN      NumStrings;
  UINT32     Attributes;
  UINTN      StringIndex;
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestPersistentStoreWrite\n"));

  NumVariables = NumNonVolatileVariables ();
  NumStrings = NumRandomStrings ();

  Attributes = (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS |
                EFI_VARIABLE_NON_VOLATILE);

  StringIndex = 0;
  Errors = WriteAllVariables (&StringIndex, Attributes, NonVolatileVarNames, NumVariables,
                              RandomStrings, NumStrings, NULL);

  if (Errors != 0) {
    ReportError ("TestPersistentStoreWrite: %d errors.\n", Errors);
  }
}


/**
  Second half of a NV variable store test.  Read and verify a bunch of
  variables from NV storage.

**/
STATIC
VOID
TestPersistentStoreReadVerify (
  )
{
  UINTN      NumVariables;
  UINTN      NumStrings;
  UINT32     Attributes;
  UINTN     *ValueIndex;
  UINTN      I;
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestPersistentStoreReadVerify\n"));

  NumVariables = NumNonVolatileVariables ();
  NumStrings = NumRandomStrings ();

  Attributes = (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS |
                EFI_VARIABLE_NON_VOLATILE);

  ValueIndex = AllocatePool (NumVariables * sizeof (UINTN));
  ASSERT (ValueIndex != NULL);

  for (I = 0; I < NumVariables; I++) {
    ValueIndex[I] = I % NumStrings;
  }

  Errors = ReadVerifyAllVariables (Attributes, NonVolatileVarNames, NumVariables,
                                   RandomStrings, NumStrings, ValueIndex);

  if (Errors != 0) {
    ReportError ("TestPersistentStoreReadVerify: %d errors.\n", Errors);
  }

  FreePool (ValueIndex);
}

/**
  Do an append write and verify the result.

  @param  Prefix        The prefix string to emit on error messages.
  @param  Name          Name of the variable.
  @param  Attributes    Attributes of the variable.
  @param  Input         The input array.  We use this array both as the source of the
                        bytes to append, and to verify the result of the append write.
  @param  InputSize     The size of the input array.
  @param  InputOffset   The offset into the input array at which the bytes to be
                        written is to be fetched.
  @param  WriteSize     The number of bytes to write.
  @param  Output        The output array.

  @return Number of errors encountered.

**/
STATIC
UINTN
AppendWriteAndVerify (
  CHAR8     *Prefix,
  CHAR16    *Name,
  UINT32     Attributes,
  CHAR8     *Input,
  UINTN      InputSize,
  UINTN      InputOffset,
  UINTN      WriteSize,
  CHAR8     *Output
  )
{
  EFI_STATUS Status;
  EFI_STATUS OutputSize;

  Attributes |= EFI_VARIABLE_APPEND_WRITE;

  Status = gRT->SetVariable (Name, &gEfiGlobalVariableGuid, Attributes,
                             WriteSize, &Input[InputOffset]);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: %s append write failed at offset %d: %r\n",
            Prefix, Name, InputOffset, Status));
    return 1;
  }

  SetMem (Output, InputSize, 0);
  OutputSize = InputSize;
  Status = gRT->GetVariable (Name, &gEfiGlobalVariableGuid, NULL, &OutputSize, Output);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: %s read failed at offset %d: %r\n",
            Prefix, Name, InputOffset, Status));
    return 1;
  }

  if (OutputSize != InputOffset + WriteSize) {
    DEBUG ((EFI_D_ERROR,
            "%a: %s size mismatch at offset %d: expect %d got %d\n",
            Prefix, Name, InputOffset, InputOffset + WriteSize, OutputSize));
    return 1;
  }

  if (CompareMem (Input, Output, OutputSize) != 0) {
    DEBUG ((EFI_D_ERROR, "%a: %s contents mismatch at offset %d\n",
            Prefix, Name, InputOffset));
    return 1;
  }

  return 0;
}

/**
  Test append write for variables of a given set of attributes.

  Set up some number of variables, and iteratively append to each
  variable a chunk at a time.  Each variable is set up with a
  different, constant chunk size varying from 1 up to 1024.

  Also specially test append of 0 bytes.

  @param  Attributes    The attributes of the variables to test.
  @param  Names         The array of variable names to use.

  @param  NumNames      Number of names available.

**/
STATIC
VOID
TestAppendWriteVariables (
  UINT32     Attributes,
  CHAR16   **Names,
  UINTN      NumNames
  )
{
  UINTN      MaxVariableSize;
  UINTN      VariableStoreSize;
  UINTN      NumVariables;
  UINTN     *ChunkSizeArray;
  UINTN      ChunkSize;
  CHAR8     *Input;
  CHAR8     *Output;
  UINTN     *TotalSizeArray;
  CHAR16    *Name;
  UINTN      Iterations;
  UINTN      Index;
  UINTN      Index2;
  UINTN      Errors;

  DEBUG ((EFI_D_INFO, "TestAppendWriteVariables %d\n", Attributes));

  // Limit each variable data size to at most 1024 bytes.
  MaxVariableSize =
    MIN (PcdGet32 (PcdMaxVariableSize) - sizeof (AUTHENTICATED_VARIABLE_HEADER) -
         StrSize (Names[0]), 1024);

  // Make sure the variables do not overflow the store.
  VariableStoreSize = PcdGet32 (PcdVariableStoreSize);
  NumVariables = MIN (NumNames, VariableStoreSize / MaxVariableSize - 2);

  ChunkSizeArray = (UINTN *)AllocatePool (NumVariables * sizeof(UINTN));
  ASSERT (ChunkSizeArray != NULL);
  TotalSizeArray = (UINTN *)AllocatePool (NumVariables * sizeof(UINTN));
  ASSERT (TotalSizeArray != NULL);

  // Set up chunk sizes of 1024, 512, ..., 2, 1.
  for (Index = 0; Index < NumVariables; Index++) {
    ChunkSizeArray[Index] = 1024 >> Index;
    TotalSizeArray[Index] = 0;
  }
  ChunkSizeArray[NumVariables - 2] = 2;
  ChunkSizeArray[NumVariables - 1] = 1;

  Input = (CHAR8 *)AllocatePool (MaxVariableSize);
  ASSERT (Input != NULL);
  Output = (CHAR8 *)AllocatePool (MaxVariableSize);
  ASSERT (Output != NULL);

  for (Index = 0; Index < MaxVariableSize - 1; Index++) {
    Input[Index] = 'A' + (Index % 63);
  }

  Input[Index] = '\0';

  // Run enough iterations to guarantee a reclaim.
  Iterations = VariableStoreSize / MaxVariableSize;

  Errors = 0;
  for (Index = 0; Index < Iterations; Index++) {
    for (Index2 = 0; Index2 < NumVariables; Index2++) {
      Name = Names[Index2];
      ChunkSize = ChunkSizeArray[Index2];
      if (TotalSizeArray[Index2] + ChunkSize > MaxVariableSize) {
        ChunkSize = 0;
      }

      Errors += AppendWriteAndVerify ("TestAppendWriteVariables", Name, Attributes,
                                      Input, MaxVariableSize,
                                      TotalSizeArray[Index2], ChunkSize, Output);
      TotalSizeArray[Index2] += ChunkSize;

      DEBUG ((EFI_D_INFO, "TestAppendWriteVariables iter %d var %d chunk size %d size %d\n",
              Index, Index2, ChunkSize, TotalSizeArray[Index2]));
    }
  }

  // Test append write of 0 byte.
  for (Index2 = 0; Index2 < NumVariables; Index2++) {
    Name = Names[Index2];

    DEBUG ((EFI_D_INFO, "TestAppendWriteVariables var %d chunk size 0 size %d\n",
            Index2, TotalSizeArray[Index2]));

    Errors += AppendWriteAndVerify ("TestAppendWriteVariables0", Name, Attributes,
                                    Input, MaxVariableSize,
                                    TotalSizeArray[Index2], 0, Output);
  }

  if (Errors != 0) {
    ReportError ("TestAppendWriteVariables %d: %d errors.\n", Attributes, Errors);
  }

  FreePool (ChunkSizeArray);
  FreePool (TotalSizeArray);
  FreePool (Input);
  FreePool (Output);
}

/**
  Unit test for append writes.

**/
STATIC
VOID
TestAppendWrite (
  )
{
  DEBUG ((EFI_D_INFO, "TestAppendWrite\n"));

  TestAppendWriteVariables (VARIABLE_ATTRIBUTE_BS_RT, VolatileVarNames,
                            NumVolatileVariables ());
  TestAppendWriteVariables (VARIABLE_ATTRIBUTE_NV_BS_RT, NonVolatileVarNames,
                            NumNonVolatileVariables ());
}

/**
  This function runs some unit tests, based on the value of
  PcdDefaultUefiTests which is overridable by the boot param file
  system.

**/
VOID
RunTests (
  )
{
  CHAR16*  Tests = (CHAR16 *)BPO_PcdGetPtr (PcdDefaultUefiTests);

  if (StrCmp (Tests, L"") == 0) {
    return;
  } else if (StrCmp (Tests, L"v1") == 0) {
    Testv1 ();
  } else if (StrCmp (Tests, L"persistent_write") == 0) {
    TestPersistentStoreWrite ();
  } else if (StrCmp (Tests, L"persistent_read") == 0) {
    TestPersistentStoreReadVerify ();
  } else if (StrCmp (Tests, L"outage_write1") == 0) {
    TestOutageWrite1 ();
  } else if (StrCmp (Tests, L"outage_write2") == 0) {
    TestOutageWrite2 ();
  } else if (StrCmp (Tests, L"outage_write3") == 0) {
    TestOutageWrite3 ();
  } else if (StrCmp (Tests, L"outage_read1") == 0) {
    TestOutageRead1 ();
  } else if (StrCmp (Tests, L"outage_read2") == 0) {
    TestOutageRead2 ();
  } else if (StrCmp (Tests, L"outage_read3") == 0) {
    TestOutageRead3 ();
  } else if (StrCmp (Tests, L"append_write") == 0) {
    TestAppendWrite ();
  } else {
    ReportError ("Unknown test %s\n", Tests);
  }
}
