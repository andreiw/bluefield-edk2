#include <Library/BaseLib.h>

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
  );

/**
  Return the number of test non-volatile variables.

  @return The number of test non-volatile variables.

**/
UINTN
NumVolatileVariables (
  );

/**
  Return the number of test non-volatile variables.

  @return The number of test non-volatile variables.

**/
UINTN
NumNonVolatileVariables (
  );

/**
  Return the number of test random strings.

  @return The number of test random strings.

**/
UINTN NumRandomStrings (
  );

/**
  Return the size of the largest string in RandomStrings.

  @return The size of the largest string in RandomStrings.

**/
UINTN GetMaxRandomStringSize (
  );

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
  );

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
  );

/**
  Verify that the value of a variable matches the expected value.

  @param  Name           Name of the variable.
  @param  ExpectedValue  Expected value of the variable.

**/
UINTN
ReadVerifyVariable (
  IN CHAR16       *Name,
  IN CHAR16       *ExpectedValue
  );

extern CHAR16 *VolatileVarNames[];
extern CHAR16 *NonVolatileVarNames[];
extern CHAR16 *RandomStrings[];
