/**
 * libtween
 * Copyright (C) 2013 libtween authors.
 *
 * Based on tween.js Copyright (c) 2010-2012 Tween.js authors.
 * Easing equations Copyright (c) 2001 Robert Penner http://robertpenner.com/easing/
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tween.h"

/**
 * Tween_CreateEngine()
 */

Tween_Engine* Tween_CreateEngine() {
    Tween_Engine* engine;

    engine = (Tween_Engine*) malloc(sizeof(Tween_Engine));
    memset(engine, 0, sizeof(Tween_Engine));
    
    return engine;
}

/**
 * Tween_DestroyEngine()
 */

void Tween_DestroyEngine(Tween_Engine* engine) {
    Tween_Node* node;

    node = engine->tweens;
    
    while (node) {
        Tween_Node* nextNode = node->next;
        free(node);
        node = nextNode;
    }
    
    free(engine);
}

/**
 * Tween_UpdateEngine()
 */

int Tween_UpdateEngine(Tween_Engine* engine, uint32_t time) {
    Tween_Node* node;
    Tween_Node* tempNode = NULL;

    node = engine->tweens;

    if (!node) {
        return 0;
    }
    
    while (node) {
        if (!Tween_UpdateTween(node->tween, time)) {
            if (tempNode) {
                tempNode->next = node->next;
                free(node);
                node = tempNode->next;
            }
            else {
                engine->tweens = node->next;
                free(node);
                node = engine->tweens;
            }
        }
        else {
            tempNode = node;
            node = node->next;
        }
    }
    
    return 1;
}

/**
 * Tween_CreateTween()
 */

Tween* Tween_CreateTween(Tween_Engine* engine, Tween_Props* props, Tween_Props* toProps, uint32_t duration, Tween_Easing easing, Tween_Callback updateCallback, void* data) {
    Tween* tween;

    tween = (Tween*) malloc(sizeof(Tween));
    memset(tween, 0, sizeof(Tween));
    
    tween->engine = engine;
    
    Tween_CopyProps(props, &tween->props);
    Tween_CopyProps(toProps, &tween->toProps);

    Tween_CopyProps(props, &tween->startProps);
    Tween_CopyProps(props, &tween->repeatProps);

    tween->duration = duration;
    tween->easing = easing;
    tween->updateCallback = updateCallback;
    tween->data = data;
    
    return tween;
}

/**
 * Tween_CreateTweenEx()
 */

Tween* Tween_CreateTweenEx(Tween_Engine* engine, Tween_Props* props, Tween_Props* toProps, uint32_t duration, uint32_t delay, int repeat, int yoyo, Tween_Easing easing, Tween_Callback startCallback, Tween_Callback updateCallback, Tween_Callback completeCallback, void* data) {
    Tween* tween;

    tween = Tween_CreateTween(engine, props, toProps, duration, easing, updateCallback, data);
    
    tween->delay = delay;
    tween->repeat = repeat;
    tween->yoyo = yoyo;
    tween->startCallback = startCallback;
    tween->completeCallback = completeCallback;
    
    return tween;
}

/**
 * Tween_ChainTweens()
 */

void Tween_ChainTweens(Tween* tween, Tween* tween2) {
    Tween_Node* node;
    Tween_Node* tempNode;

    node = (Tween_Node*) malloc(sizeof(Tween_Node));
    memset(node, 0, sizeof(Tween_Node));

    node->tween = tween2;
    
    if (tween->chain == NULL) {
        tween->chain = node;
    }
    else {
        tempNode = tween->chain;

        while(tempNode->next) {
            tempNode = tempNode->next;
        }

        tempNode->next = node;
    }
}

/**
 * Tween_DestroyTween()
 */

void Tween_DestroyTween(Tween* tween) {
    Tween_Node* node;
    Tween_Node* tempNode;

    node = tween->chain;
    
    while (node) {
        tempNode = node->next;
        free(node);

        node = tempNode;
    }
    
    free(tween);
}

/**
 * Tween_StartTween()
 */

void Tween_StartTween(Tween* tween, uint32_t time) {
    Tween_Node* node;
    Tween_Node* tempNode;

    tween->startTime = time;
    tween->startTime += tween->delay;
    
    Tween_CopyProps(&tween->startProps, &tween->props);
    
    node = (Tween_Node*) malloc(sizeof(Tween_Node));
    memset(node, 0, sizeof(Tween_Node));

    node->tween = tween;
    
    if (!tween->engine->tweens) {
        tween->engine->tweens = node;
    }
    else {
        tempNode = tween->engine->tweens;

        while(tempNode->next) {
            tempNode = tempNode->next;
        }

        tempNode->next = node;
    }
}

/**
 * Tween_StopTween()
 */

void Tween_StopTween(Tween* tween) {
    Tween_Node* node;
    Tween_Node* tempNode = NULL;

    node = tween->engine->tweens;
    
    while (node) {
        if (node->tween == tween) {
            if (tempNode) {
                tempNode->next = node->next;
            }
            else {
                tween->engine->tweens = node->next;
            }
            
            free(node);
            
            break;
        }
        else {
            tempNode = node;
            node = node->next;
        }
    }
}

/**
 * Tween_UpdateTween()
 */

int Tween_UpdateTween(Tween* tween, uint32_t time) {
    double elapsed; 
    Tween_Node* node;

    if (time < tween->startTime) {
        return 1;
    }
    
    if (!tween->startCallbackFired) {
        if (tween->startCallback) {
            tween->startCallback(tween);
        }
        
        tween->startCallbackFired = 1;
    }
    
    elapsed = (double)(time - tween->startTime) / (double)tween->duration;
    elapsed = (elapsed > 1) ? 1 : elapsed;
    
    Tween_UpdateProps(&tween->startProps, &tween->toProps, &tween->props, tweenEasingFuncs[tween->easing](elapsed));
    
    if (tween->updateCallback) {
        tween->updateCallback(tween);
    }
    
    if (elapsed == 1) {
        if (tween->repeat > 0) {
            tween->repeat--;
            
            if (tween->yoyo) {
                Tween_SwapProps(&tween->repeatProps, &tween->toProps);
                tween->reversed = !tween->reversed;
            }
            
            Tween_CopyProps(&tween->repeatProps, &tween->startProps);
            tween->startTime = time + tween->delay;
            
            return 1;
        }
        else {
            node = tween->chain;
            
            while (node) {
                Tween_StartTween(node->tween, time);
                node = node->next;
            }
            
            if (tween->completeCallback) {
                tween->completeCallback(tween);
            }
            
            return 0;
        }
    }
    
    return 1;
}
