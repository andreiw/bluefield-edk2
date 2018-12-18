/** @file

  Copyright (c) 2018, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license
  may be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/BaseMemoryLib.h>

#include "I2cSmbusRuntimeDxe.h"
#include "I2cSmbusIpmb.h"

// IPMB global context.
IPMB_CONTEXT gIpmbContext;

// I2C-based request packet.
STATIC I2C_SMBUS_REQUEST_PACKET mRequestPacket;

// IPMB sequence number. This is used to verify that a response is for
// a particular instance of a request.
STATIC UINT8 gSequenceNumber = 0;

// Encapsulates the parameters needed by the backend slave function
// when invoked by the I2C SMBUS Slave send/receive handlers.
typedef struct {
  UINT8     *WrDataPtr;
  UINT32    WrDataLength;
  UINT32    WrDataActualLength;
  BOOLEAN   WrParamsSet;
} IPMB_SLAVE_FUNCTION_PARAMS;

STATIC IPMB_SLAVE_FUNCTION_PARAMS gIpmbSlaveFunctionParams;

// This function sets the slave function parameters. It MUST be called prior
// any call to I2C slave handlers.
STATIC
VOID
I2cSmbusIpmbSetSlaveFunctionParams (
  IN     UINT8                *WrDataPtr,
  IN     UINT32               WrDataLength,
  IN     BOOLEAN              WrParamsSet
  )
{
  gIpmbSlaveFunctionParams.WrDataPtr    = WrDataPtr;
  gIpmbSlaveFunctionParams.WrDataLength = WrDataLength;
  gIpmbSlaveFunctionParams.WrParamsSet  = WrParamsSet;
}

// IPMB backend function. This functions consists of a slave FSM to handle
// I2C master requests.
STATIC
EFI_STATUS
I2cSmbusIpmbSlaveFunction (
  IN        UINT8  SlaveAddress,
  IN        UINT8  *Data,
  IN        UINT8  DataLength,
  IN        UINT32 Flags,
  IN OUT    UINT8  *ByteTransferred
)
{
  if (SlaveAddress != gIpmbContext.SlaveAddress) {
    I2cSmbusIpmbSetSlaveFunctionParams (NULL, 0, FALSE);
    return EFI_ACCESS_DENIED;
  }

  if (ByteTransferred)
    *ByteTransferred = 0;

  if (gIpmbSlaveFunctionParams.WrParamsSet == FALSE)
    return EFI_NOT_READY;

  switch (Flags) {
  case SMBUS_SLAVE_WRITE_REQUESTED:
    if (DataLength > gIpmbSlaveFunctionParams.WrDataLength)
      return EFI_BAD_BUFFER_SIZE;
    CopyMem (gIpmbSlaveFunctionParams.WrDataPtr, Data, DataLength);
    break;

  case SMBUS_SLAVE_STOP:
    I2cSmbusIpmbSetSlaveFunctionParams (NULL, 0, FALSE);
    break;

  case SMBUS_SLAVE_READ_REQUESTED: // Master is not allowed to read from us.
    if (ByteTransferred)
      *ByteTransferred = 0;
  default:
    return EFI_UNSUPPORTED;
  }

  if (ByteTransferred)
    *ByteTransferred = DataLength;

  return EFI_SUCCESS;
}

// Start the slave device; this marks the device as active. After this call
// the device is able to receive responses.
STATIC
EFI_STATUS
I2cSmbusIpmbSlaveStart (
  IN        UINT8  SlaveAddress
)
{
  return I2cSmbusSlaveEnableAddress (gIpmbContext.I2cSmbus, SlaveAddress);
}

// Stop the slave device; this marks the device as inactive. After this call
// the device is no longer able to receive responses.
STATIC
EFI_STATUS
I2cSmbusIpmbSlaveStop (
  IN        UINT8  SlaveAddress
)
{
  return I2cSmbusSlaveDisableAddress (gIpmbContext.I2cSmbus, SlaveAddress);
}

/**
  This service enables IPMB controller to recieve responses via I2C SMBus.

  @param[in]         Address            I2C Address of the SMBUS Slave device.
  @param[in]         IpmbResponse       Ipmi Response Packet.
  @param[in]         IpmbResponseSize   Size of Ipmi Response Packet.
  @param[in][out]    ResponseData       Response Data.
  @param[in][out]    IpmbRequestSize    Size of Response Data.

  @retval EFI_SUCCESS            The response byte stream was successfully
                                 received.
  @retval EFI_NOT_FOUND          Ipmi device failed to retreive the SMBUS
                                 slave device.
  @retval EFI_DEVICE_ERROR       Ipmi Device hardware error.
  @retval EFI_TIMEOUT            I2c device time out.
  @retval EFI_UNSUPPORTED        The I2c bytes was not successfully sent to
                                 the device.
  @retval EFI_OUT_OF_RESOURCES   Data size error.
**/
STATIC
EFI_STATUS
I2cSmbusIpmbWaitForResponse (
  IN     UINT8                Address,
  IN     IPMB_RESPONSE_PACKET *IpmbResponse,
  IN     UINT32               IpmbResponseSize,
  IN OUT UINT8                *ResponseData,
  IN OUT UINT32               *ResponseDataSize
  )
{
  EFI_STATUS Status;
  UINTN      ByteReceived = 0;

  I2cSmbusIpmbSetSlaveFunctionParams (
              (UINT8 *) IpmbResponse,
              IpmbResponseSize,
              TRUE
              );

  // Executes the slave receive handler to read the received I2C data.
  Status = I2cSmbusSlaveReceive (
                Address,
                &ByteReceived,
                I2cSmbusIpmbSlaveFunction
                );

  if (EFI_ERROR (Status))
    return Status;

  if (ByteReceived < MIN_IPMB_RESPONSE_SIZE)
    return EFI_OUT_OF_RESOURCES;

  IpmbResponse->RqSa >>= 1;
  // Copy out the the Completion Code and the Data bytes from the IPMB
  // response. The response data doesn't include the checksum byte.
  *ResponseDataSize  = ByteReceived - MIN_IPMB_RESPONSE_SIZE;
  *ResponseDataSize += 1; // Prepend the Completion Code.
  if (*ResponseDataSize)
    CopyMem (ResponseData, &IpmbResponse->CompCode, *ResponseDataSize);

  return EFI_SUCCESS;
}

/**
  This service enables IPMB controller to send requests via I2C SMBus.

  @param[in]         IpmbRequest       Ipmi Request Packet.
  @param[in]         IpmbRequestSize   Size of Ipmi Request Packet.

  @retval EFI_SUCCESS            The request byte stream was successfully
                                 sent to the management controller.
  @retval EFI_DEVICE_ERROR       Ipmi Device hardware error.
**/
STATIC
EFI_STATUS
I2cSmbusIpmbSendRequest (
  IN IPMB_REQUEST_PACKET *IpmbRequest,
  IN UINT32              IpmbRequestSize
  )
{
  EFI_STATUS                Status;
  EFI_SMBUS_DEVICE_ADDRESS  DeviceAddress;
  EFI_SMBUS_OPERATION       SmbusOperation;
  UINTN                     BufferLength;
  UINT8                     Buffer[MAX_IPMB_PACKET_SIZE];

  // Determine the size of the I2C request buffer; This buffer must
  // contains all the I2C data that needs to be sent except the slave
  // address byte and the command byte.
  BufferLength = MIN_IPMB_PACKET_SIZE - 2 + IpmbRequestSize;
  // Copy out the IPMB packet and skip the two first bytes; the first
  // byte holds the responder address (slave address) and the second
  // byte holds the netFn (command set).
  CopyMem (Buffer, &IpmbRequest->ChkSum1, BufferLength);

  mRequestPacket.OperationCount              = 1;
  mRequestPacket.Operation[0].Flags          = 0;
  mRequestPacket.Operation[0].Buffer         = Buffer;
  mRequestPacket.Operation[0].LengthInBytes  = BufferLength;

  // Request Messages are transmitted on the bus using I2C Master Write
  // transfers.
  SmbusOperation                   = EfiSmbusWriteBlock;
  DeviceAddress.SmbusDeviceAddress = IpmbRequest->RsSa; // Set slave address.

  // Executes an SMBus operation to an SMBus controller.
  Status = gIpmbContext.I2cSmbusExecute (
                              NULL,
                              DeviceAddress,
                              IpmbRequest->NfLn,
                              SmbusOperation,
                              FALSE,
                              &BufferLength,
                              (VOID *) &mRequestPacket
                              );

  return (EFI_ERROR (Status)) ? EFI_DEVICE_ERROR: EFI_SUCCESS;
}

//
// IPMI protocol implementation
//

EFI_STATUS
EFIAPI
I2cSmbusIpmiSubmitCommand (
  IN     IPMI_PROTOCOL      *This,
  IN     UINT8              NetFunction,
  IN     UINT8              Command,
  IN     UINT8              *RequestData,
  IN     UINT32             RequestDataSize,
     OUT UINT8              *ResponseData,
  IN OUT UINT32             *ResponseDataSize
  )
{
  EFI_STATUS           Status;
  IPMB_REQUEST_PACKET  IpmbRequest  = { 0 };
  IPMB_RESPONSE_PACKET IpmbResponse = { 0 };
  UINT8                RetryCnt     = 0;
  UINT8                DataSize     = 0;
  UINT8                RsChecksum1, RsChecksum2;

  // Request data size shouldn't include the checksum byte. Thus, offset
  // 'RequestDataSize' of IpmbRequest.Data buffer must contain a checksum
  // byte.
  if (RequestDataSize > MAX_IPMB_PACKET_DATA_SIZE)
    return EFI_OUT_OF_RESOURCES;

  //
  // Prepare the IPMB request.
  //

  IpmbRequest.RsSa    = gIpmbContext.BmcAddress;
  IpmbRequest.NfLn    = NetFunction;
  // Calculate the checksum byte of the connection header.
  IpmbRequest.ChkSum1 = CalculateCheckSum8 ((UINT8 *) &IpmbRequest, 2);
  IpmbRequest.RqSa    = gIpmbContext.SlaveAddress;
  IpmbRequest.SeqLn   = gSequenceNumber++;
  IpmbRequest.Cmd     = Command;
  CopyMem(IpmbRequest.Data, RequestData, RequestDataSize);

  // Calculate packet checksum and append it to the end of the data buffer.
  // Note that we calculate the checksum over the entire packet.
  IpmbRequest.Data[RequestDataSize] =
      CalculateCheckSum8 ((UINT8 *) &IpmbRequest,
                          MIN_IPMB_PACKET_SIZE + RequestDataSize - 1);

  // Start the Ipmb device. This call shouldn't fail since the Ipmb protocol
  // has been installed successfully.
  Status = I2cSmbusIpmbSlaveStart (IpmbRequest.RqSa);
  ASSERT (Status == EFI_SUCCESS);

  RetryCnt = PcdGet8 (PcdIpmbRetryCnt);
  if (RetryCnt == 0)
    RetryCnt = MIN_IPMB_RETRY;

  // Retry if the request failed, or if we failed to receive a response.
  while (RetryCnt) {
    // Send the IPMB request.
    Status = I2cSmbusIpmbSendRequest (&IpmbRequest,
                                      MIN_IPMB_PACKET_SIZE + RequestDataSize);

    if (!EFI_ERROR (Status)) {
      // Now check whether a response is available.
      Status = I2cSmbusIpmbWaitForResponse (
                        IpmbRequest.RqSa,
                        &IpmbResponse,
                        MAX_IPMB_RESPONSE_SIZE,
                        ResponseData,
                        ResponseDataSize
                        );
    }

    if (!EFI_ERROR (Status))
      break;

    RetryCnt--;
    // Update the packet sequence number to renew the request.
    IpmbRequest.SeqLn = gSequenceNumber++;
  }

  // Stop the Ipmb device. This call shouldn't fail since the Ipmb device
  // start has been installed successfully.
  I2cSmbusIpmbSlaveStop (IpmbRequest.RqSa);

  // Return if the Ipmb command failed.
  if (EFI_ERROR (Status))
    return Status;

  // Response data size doesn't include the checksum byte. However it
  // includes the Completion Code byte. Thus the checksum byte should
  // be at offset 'ResponseDataSize - 1' of IpmbResponse Data buffer.
  DataSize = *ResponseDataSize - 1;
  if (DataSize > MAX_IPMB_RESPONSE_DATA_SIZE)
    return EFI_BAD_BUFFER_SIZE;

  //
  // Process the IPMB response.
  //

  // Calculate checksum bytes. Note there are two checkum bytes, one in
  // the connection header and a second one is appended to the end of the
  // data.
  RsChecksum1 = CalculateCheckSum8 ((UINT8 *) &IpmbResponse, 2);
  // Note that we calculate the checksum over the entire packet; except
  // we don't include the last byte, i.e., the checksum byte.
  RsChecksum2 = CalculateCheckSum8 (
                    (UINT8 *) &IpmbResponse,
                    (MIN_IPMB_RESPONSE_SIZE + DataSize - 1)
                    );

  // Note that the Completion Code MUST be verified by the IPMI command
  // handler. Here we verify wehther the response is valid only, we don't
  // verify whether the command is supported or not.
  if (IpmbResponse.SeqLn    != IpmbRequest.SeqLn ||
       IpmbResponse.RsSa    != IpmbRequest.RsSa  ||
       IpmbResponse.NfLn    != IpmbRequest.NfLn  ||
       IpmbResponse.Cmd     != IpmbRequest.Cmd   ||
       IpmbResponse.ChkSum1 != RsChecksum1       ||
       IpmbResponse.Data[DataSize] != RsChecksum2) {
    return EFI_PROTOCOL_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  This service enables the SMBUS Slave device and installs the IPMI protocol.
**/
EFI_STATUS
EFIAPI
I2cSmbusInstallIpmbProtocol (
  VOID
  )
{
  EFI_STATUS      Status = EFI_SUCCESS;
  UINT8           Bus, SlaveAddress, BmcAddress;

  ASSERT (gSmbusContext.SmbusHc.Execute != NULL);

  // Read IPMB config parameters.
  Bus          = PcdGet8 (PcdIpmbBusId);
  SlaveAddress = PcdGet8 (PcdIpmbSlaveAddress);
  BmcAddress   = PcdGet8 (PcdIpmbBmcAddress);

  gIpmbContext.Signature                      = IPMB_SIGNATURE;
  gIpmbContext.IpmiProtocol.IpmiSubmitCommand = I2cSmbusIpmiSubmitCommand;
  gIpmbContext.I2cSmbusExecute                = gSmbusContext.SmbusHc.Execute;
  gIpmbContext.I2cSmbus                       = Bus;
  gIpmbContext.SlaveAddress                   = SlaveAddress;
  gIpmbContext.BmcAddress                     = BmcAddress;

  // Register the management controller BMC as a slave device.
  Status = I2cSmbusRegisterSlaveDevice (Bus, BmcAddress);
  if (EFI_ERROR (Status))
    goto out;

  // Initialize the Smbus slave device.
  Status = I2cSmbusSlaveInitialize (Bus, SlaveAddress);
  if (EFI_ERROR (Status))
    goto out;

  Status = gBS->InstallMultipleProtocolInterfaces (
      &gIpmbContext.ControllerHandle,
      &gIpmiProtocolGuid,
      &gIpmbContext.IpmiProtocol,
      NULL
      );

out:
  DEBUG ((DEBUG_ERROR,
        "I2cSmbusInstallIpmiProtocol: %r\n", Status));
  return Status;
}
