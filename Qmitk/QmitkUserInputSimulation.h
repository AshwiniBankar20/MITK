/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkUserInputSimulationMacroshincluded
#define QmitkUserInputSimulationMacroshincluded

#include <qwidget.h>

class QmitkUserInputSimulation
{
  public:

    /// See documentation of QMouseEvent for values of \param button
    static void MouseDown( QWidget* widget, int button );
    
    static void MouseDownXY( QWidget* widget, int x, int y, int button );

    static void MouseMoveXY( QWidget* widget, int x, int y, int state );
    static void CtrlDragMouseXY( QWidget* widget, int x, int y, int mouseButton );

    static void MouseRelease( QWidget* widget, int button );
    
    static void MouseReleaseXY( QWidget* widget, int x, int y, int button );
    
    static void MouseClick( QWidget* widget, int button );

    static void MouseMoveScrollWheel( QWidget* widget, int delta );
    static void MouseMoveScrollWheel( QWidget* widget, int x, int y, int delta );

    static void MouseDrawRandom( QWidget* widget, int button, unsigned int points = 100 );
    static void MouseDrawCircle( QWidget* widget, int button, float relativePositionX = 0.5, float relativePositionY = 0.5, float relativeRadius = 0.3 );
    static void MouseDrawLine( QWidget* widget, int button, Qt::Orientation orientation );
 
    static void SimulateKeyboardTyping( QWidget* widget, const QString& text );
    static void KeyboardTypeKey( QWidget* widget, char c );
    static void KeyboardKeyDown( QWidget* widget, char c );
    static void KeyboardKeyRelease( QWidget* widget, char c );

  protected:

};

#endif

