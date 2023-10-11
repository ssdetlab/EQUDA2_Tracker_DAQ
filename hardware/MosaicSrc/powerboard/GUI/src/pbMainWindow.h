/*
 * Copyright (C) 2016
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2016.
 *
 */
#ifndef PBMAINWINDOW_H
#define PBMAINWINDOW_H

#include "pbif.h"
#include "powerboard.h"
#include "setValidator.h"
#include <QCheckBox>
#include <QDomElement>
#include <QLCDNumber>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPixmap>
#include <QSignalMapper>
#include <QTimer>

#ifdef PB_NEW
#define NUM_TSENSORS 3
#else
#define NUM_TSENSORS 1
#endif
#define NUM_MODULES 8
#define NUM_CHANNELS 16

class pbMainWindow : public QMainWindow {
  Q_OBJECT

public:
  pbMainWindow(QWidget *parent = 0, Qt::WindowFlags fl = 0);
  ~pbMainWindow();

  void setIPaddress(QString add);

private:
  QWidget *      centralWidgetPtr;
  QString        cfgFilename;
  QPixmap        ledRedPixmap;
  QPixmap        ledGreyPixmap;
  QLCDNumber *   temperatureLCD[NUM_TSENSORS];
  QCheckBox *    chVbiasOn[NUM_MODULES];
  QLabel *       VbiasLED;
  QLabel *       channelLED[NUM_CHANNELS];
  bool           channelON[NUM_CHANNELS];
  QLCDNumber *   VbiasLCD;
  QLCDNumber *   IbiasLCD;
  QLCDNumber *   VoltLCD[NUM_CHANNELS];
  QLCDNumber *   AmpLCD[NUM_CHANNELS];
  QLineEdit *    VsetText[NUM_CHANNELS];
  QLineEdit *    IsetText[NUM_CHANNELS];
  QLineEdit *    VbiasText;
  QSignalMapper *ChannelSetOnMapper;
  QSignalMapper *ChannelSetOffMapper;
  QSignalMapper *ChannelVsetMapper;
  QSignalMapper *ChannelIsetMapper;
  QSignalMapper *ChannelBiasMapper;
  QTimer *       refreshTimer;
  setValidator * VbiasValidator;
  setValidator * VsetValidator;
  setValidator * IsetValidator;
  PBif *         board;
  powerboard *   pb;
  bool           boardIsOnline;
  QString        boardAddress;

public slots:
  void fileOpen(char *fname = 0);

private slots:
  void channelSetON(int ch);
  void channelSetOFF(int ch);
  void channelVset(int ch);
  void channelIset(int ch);
  void biasCheckBoxChanged(int ch);
  void storeVset();
  void VbiasSet();
  //	void enAllVbias(bool en);
  void enVbias(bool en, int ch);
  void refreshMonitor();
  void refreshSettings();
  void allON();
  void allOFF();

  void saveConfiguration();
  void fileSaveAs();
  void fileSave();
  void configure();

private:
  QWidget *   topStatusBar();
  QWidget *   channel(int ch, QString chName);
  QLCDNumber *newLargeLCD();
  QLCDNumber *newSmallLCD();
  bool        XMLreadChannel(QDomElement &root, int n);
  bool        XMLreadBias(QDomElement &root);
  bool        XMLreadMOSAIC(QDomElement &root);
  QDomElement XMLgetTag(QDomElement &e, QString tagName);
  void        comErrorExit(std::exception &e);
  void        setOnline(bool online);
};

#endif // PBMAINWINDOW
