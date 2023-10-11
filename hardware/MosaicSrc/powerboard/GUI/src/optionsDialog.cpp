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

#include "optionsDialog.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

static const char *WINDOW_TITLE = "Option editor";

optionsDialog::optionsDialog(QWidget *parent, Qt::WindowFlags fl) : QDialog(parent, fl)
{
  setWindowTitle(WINDOW_TITLE);
  setModal(true);

  QVBoxLayout *verticalLayout = new QVBoxLayout(this);

  QLabel *l = new QLabel("MOSAIC board address", this);
  verticalLayout->addWidget(l);

  addressText = new QLineEdit(this);
  addressText->setText("0.0.0.0");
  verticalLayout->addWidget(addressText);

  //	verticalLayout->setContentsMargins(0,0,0,0);

  QDialogButtonBox *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  verticalLayout->addWidget(buttonBox);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

optionsDialog::~optionsDialog() {}
