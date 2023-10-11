/*
 * Copyright (C) 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * ====================================================
 *     __  __   __  _____  __   __
 *    / / /  | / / / ___/ /  | / / SEZIONE di BARI
 *   / / / | |/ / / /_   / | |/ /
 *  / / / /| / / / __/  / /| / /
 * /_/ /_/ |__/ /_/    /_/ |__/
 *
 * ====================================================
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2017.
 *
 */
#include "pbMainWindow.h"
#include <QApplication>
#include <QMessageBox>
#include <string.h>

pbMainWindow *theMainWindow;

#define PROGRAM_VERSION "Ver. 1.1.0"

int main(int argc, char **argv)
{
  char cfgFileName[2000];

  QApplication *a;
  a = new QApplication(argc, argv);

  cfgFileName[0] = '\0';
  std::string ipAddress;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-cfg") == 0) {
      if ((i + 1) >= argc)
        goto invokeError;
      else
        strncpy(cfgFileName, argv[++i], 2000 - 1);
    }
    else if (strcmp(argv[i], "-ip") == 0) {
      if ((i + 1) >= argc)
        goto invokeError;
      else
        ipAddress = argv[++i];
    }
  }

  theMainWindow = new pbMainWindow();
  theMainWindow->show();
  if (cfgFileName[0]) theMainWindow->fileOpen(cfgFileName);
  if (!ipAddress.empty()) theMainWindow->setIPaddress(ipAddress.data());
  return a->exec();

invokeError:
  QMessageBox::critical(0, "pbGUI", "Parameters parsing Error", "Exit");
  qDebug("Parameters reading error");
  qDebug("use:pbGUI -cfg file.cfg");
  return 1;
}
