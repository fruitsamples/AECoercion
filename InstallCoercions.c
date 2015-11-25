/*------------------------------------------------------------------------------**  Apple Developer Technical Support**  AppleEvent Coercion Handler and INIT sample**  Program:    AECoercionINIT*  File: InstallCoercions.c -    C Source**  by:   C.K. Haun <TR>**  Copyright � 1991 Apple Computer, Inc.*  All rights reserved.**------------------------------------------------------------------------------* This file contains the coercion installing routine.* This will get called by our assembly stub at INIT 31 time.* Here we get space in the System Heap for our coercions (System level coercions MUST* be in the system heap) and move the code to the sys heap, then tell the * AppleEvent manager about the coercions.* For my BoolToChar, I also grab some resources from my INIT file, since it's* open now.* Of course, please read the section on coercion routines in Inside Mac VI, AppleEvents*------------------------------------------------------------------------------*//* our includes */#include <Types.h>#include <memory.h>#include <Resources.h>#include <toolutils.h>#include <AppleEvents.h>#include <GestaltEqu.h>#include <Errors.h>/* externs defining the coercion routines in Coercions.c */extern pascal OSErr CoerceCharToPString(DescType origData, Ptr inPtr, Size theSize, DescType toType, long refCon, AEDesc *result);extern pascal OSErr CoercePStringToChar(DescType origData, Ptr inPtr, Size theSize, DescType toType, long refCon, AEDesc *result);extern pascal OSErr CoerceBooleanToChar(DescType origData, Ptr inPtr, Size theSize, DescType toType, long refCon, AEDesc *result);extern void Dummy(void);/* my special AE data type */#define typeMyPString 'MPST'    /* resource IDs for my strings */#define kFalseString 128#define kTrueString 129/* For this example, I created a typePString.  A typePString would be... *//* descriptorType = 'MPST' dataHandle = (handle containing a Pascal-type string) *//* A structure I use for my BoolToChar coercion */struct myBtoCData {    Handle falseString;    Handle trueString;};typedef struct myBtoCData myBtoCData, *myBtoCDataPtr, **myBtoCDataHdl;/* This code installs all our coercion routines *//* Also, I'm checking to see if there is already a converter for this type, if there *//* is (unlikely) I do not replace it. You may want to install a hander even if *//* there is already one there, or rather you may want *//* to chain to it after you've done your work.  I would put the old coercion *//* proc pointer in the RefCon when I install my coercer, so It will always be passed *//* to me, since I'm not allocating any global storage in this example */OSErr cInstall(void){    OSErr myErr;    OSErr installErr;    long aLong;    Ptr functionStart, functionEnd, newLocation;    ProcPtr oldHandler;    long oldRefCon;    Boolean typeIsDesc;    myBtoCDataHdl stringData;    /* First thing we have to do is see if AppleEvents are installed on this machine */    /* if they are NOT, then we bail out fast.  Or at least at 8 Mhz */    /* unless we're on a PowerBook in slowdown mode, then it's slower.  But anyway */    /* fast enough */    myErr = Gestalt(gestaltAppleEventsAttr, &aLong);    if (!myErr) {        /* we're OK, the manager is installed */        /* install the first one.  Remember, you can pass any number in the refCon */        /* field during installs, that number will always get passed to your */        /* coercion routine.  It could be a procptr if your chaining, or it could */        /* be a handle if you want some permenent storage */        /* First check to see if there is one already, unlikely as all get out */        installErr = AEGetCoercionHandler(typeChar, typeMyPString, &oldHandler, &oldRefCon, &typeIsDesc, true);                /* check the error.  If the handler isn't there, we'll install our own */        /* if it is there, we don't.  You may want to, if you want to add */        /* a handler where one already exsists, PLEASE keep the information  */        /* about the old one, and chain to it when you're through.  See the AEM */        /* chapter for more details */        if ((installErr == errAEHandlerNotFound)) {            /* Get a pointer for this coercion */                        functionStart = (Ptr)CoerceCharToPString;            functionEnd = (Ptr)CoercePStringToChar;            newLocation = NewPtrSys(functionEnd - functionStart);            installErr = noErr;            if (newLocation) {                BlockMove(functionStart, newLocation, (functionEnd - functionStart));                /* Install my handler.  Remember, you can pass any longint in the refCon */                /* field here (I've left it nil) and that number will be passed to your coercion */                /* routine anytime it's called.  Maybe you want to allocate a handle */                /* and keep data in it, or something like that */                /* see the BoolToChar coercion for an example of this */                installErr = AEInstallCoercionHandler(typeChar, typeMyPString, (ProcPtr)newLocation, nil, false, true);            }            if (installErr)                DisposPtr(newLocation);        }        installErr = AEGetCoercionHandler(typeMyPString, typeChar, &oldHandler, &oldRefCon, &typeIsDesc, true);        if ((installErr == errAEHandlerNotFound)) {            functionStart = (Ptr)CoercePStringToChar;            functionEnd = (Ptr)CoerceBooleanToChar;            newLocation = NewPtrSys(functionEnd - functionStart);                        installErr = noErr;            if (newLocation) {                BlockMove(functionStart, newLocation, (functionEnd - functionStart));                installErr = AEInstallCoercionHandler(typeMyPString, typeChar, (ProcPtr)newLocation, nil, false, true);            }            if (installErr)                DisposPtr(newLocation);        }                /* This one is a bit more interesting, because I need some additional resources */        /* to make it work, two text strings.  I will create a handle to hold these, */        /* and pass that handle in the refCon so I always get it when I do the coercion */        installErr = AEGetCoercionHandler(typeBoolean, typeChar, &oldHandler, &oldRefCon, &typeIsDesc, true);        if ((installErr == errAEHandlerNotFound)) {            functionStart = (Ptr)CoerceBooleanToChar;            functionEnd = (Ptr)Dummy;            newLocation = NewPtrSys(functionEnd - functionStart);                        installErr = noErr;            if (newLocation) {                /* we got the memory for the routine, now can we get the memory for the */                /* strings and the strings themselves??? */                stringData = (myBtoCDataHdl)NewHandleSys(sizeof(myBtoCData));                if (stringData) {                                        HLock((Handle)stringData);              /* get the resources, if possible */                    /* ��� CAUTION ��� CAUTION ��� */                    /* These resources MUST be in the System heap! */                    /* I have made sure of this by marking them as "sysheap" */                    /* in their resource attributes, please see the AECoerceINIT.r file. */                    /* You can do this, or move them to the system heap after you have  */                    /* loaded them.  You MUST have them in the system heap!   */                    /* If you don't mark them as "sysheap", or don't */                    /* move them yourself, then they will be in this  */                    /* installer INIT heap, which will go away */                    /* when this INIT is finished, and the resources will also */                    /* go away! */                    /* If you want resources to stay around from an INIT, */                    /* make sure they're in the system heap!!!!! */                    (*stringData)->falseString = GetResource('STR ', kFalseString);                    if (!ResError() && (*stringData)->falseString) {                        /* remove any resource references to this handle, and  */                        /* make sure it won't get purged */                        DetachResource((*stringData)->falseString);                        HNoPurge((*stringData)->falseString);                                                (*stringData)->trueString = GetResource('STR ', kTrueString);                        if (!ResError() && (*stringData)->trueString) {                            /* remove any resource references to this handle, and  */                            /* make sure it won't get purged */                            DetachResource((*stringData)->trueString);                            HNoPurge((*stringData)->trueString);                            /* Got our strings */                            HUnlock((Handle)stringData);                            BlockMove(functionStart, newLocation, (functionEnd - functionStart));                            installErr = AEInstallCoercionHandler(typeBoolean, typeChar, (ProcPtr)newLocation, (long)stringData,                                                                  false, true);                            if (installErr) {                                /* Had an error installing the coercion */                                /* kill the memory for our resources */                                DisposHandle((*stringData)->falseString);                                DisposHandle((*stringData)->trueString);                                DisposHandle((Handle)stringData);                            }                        } else {                            /* Couldn't get the 'true' string, stop the process */                            /* and release the memory we had gotten */                            DisposHandle((*stringData)->falseString);                            DisposHandle((Handle)stringData);                            installErr = -9;                        }                    } else {                        /* couldn't get the false string, installation failed */                        DisposHandle((Handle)stringData);                        installErr = -9;                    }                }            } else {                /* string handle error */                installErr = memFullErr;            }            if (installErr)                DisposPtr(newLocation);        }    }    return(myErr);}