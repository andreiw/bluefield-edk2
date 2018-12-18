#include <Library/BaseLib.h>

/**
  First half of a test simulating a power outage in the middle of a
  persistent variable write.

**/
VOID
TestOutageWrite1 (
  );

/**
  First half of a test simulating a power outage in the middle of a
  persistent variable write.

**/
VOID
TestOutageWrite2 (
  );

/**
  First half of a test simulating a power outage in the middle of a
  persistent variable write.

**/
VOID
TestOutageWrite3 (
  );

/**
  Second half of a test simulating outage in the middle of an NVStore
  write.  Verify that the value of a variable after the simulated
  outage is as expected.

**/
VOID
TestOutageRead1 (
  );

/**
  Second half of a test simulating outage in the middle of an NVStore
  write.  Verify that the value of a variable after the simulated
  outage is as expected.

**/
VOID
TestOutageRead2 (
  );

/**
  Second half of a test simulating outage in the middle of an NVStore
  write.  Verify that the value of a variable after the simulated
  outage is as expected.

**/
VOID
TestOutageRead3 (
  );
