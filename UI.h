//
//  UI.h
//  TrajVis
//
//  Created by Diego Gonçalves on 22/05/20.
//  Copyright © 2020 Diego Gonçalves. All rights reserved.
//

#ifndef UI_h
#define UI_h

#include <stdio.h>
#include "InputManager.h"
#include "Renderable.h"

class UI : public Renderable
{
    public:
        InputController *input;
        UI();
        ~UI();
        void Render();
        void initializeShader();
};

#endif /* UI_h */
