#ifndef _USBHELPERS_H_
#define _USBHELPERS_H_

#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include <libusb-1.0/libusb.h>
#include <vector>

int  InitLibUsb();
bool IsDAQBoard(libusb_device *device);
int  AddDAQBoard(libusb_device *device, TBoardConfigDAQ *boardConfig,
                 std::vector<TReadoutBoard *> *boards);
int  FindDAQBoards(TConfig *config, std::vector<TReadoutBoard *> *boards);

#endif /* _USBHELPERS_H_ */
