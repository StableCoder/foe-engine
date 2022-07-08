// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MOUSE_HPP
#define MOUSE_HPP

#include <foe/wsi/mouse.hpp>

void mousePreprocessing(foeWsiMouse *pMouse);

void cursorPositionCallback(foeWsiMouse *pMouse, double xPos, double yPos);

void cursorEnterCallback(foeWsiMouse *pMouse, int entered);

void scrollCallback(foeWsiMouse *pMouse, double xOffset, double yOffset);

void buttonCallback(foeWsiMouse *pMouse, int button, int action, int);

#endif // MOUSE_HPP