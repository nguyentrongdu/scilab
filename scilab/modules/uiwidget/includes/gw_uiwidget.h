/*
 * Scilab ( http://www.scilab.org/ ) - This file is part of Scilab
 * Copyright (C) 2012 - Scilab Enterprises - Calixte DENIZET
 *
 * This file must be used under the terms of the CeCILL.
 * This source file is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.  The terms
 * are also available at
 * http://www.cecill.info/licences/Licence_CeCILL_V2-en.txt
 *
 */
/*--------------------------------------------------------------------------*/
#ifndef __GW_UIWIDGET_H__
#define __GW_UIWIDGET_H__
/*--------------------------------------------------------------------------*/
#include "dynlib_uiwidget_scilab.h"
/*--------------------------------------------------------------------------*/
UIWIDGET_SCILAB_IMPEXP int gw_uiwidget(void);

/*--------------------------------------------------------------------------*/
UIWIDGET_SCILAB_IMPEXP int sci_uiwidget(char *fname, unsigned long fname_len);
UIWIDGET_SCILAB_IMPEXP int sci_uiget(char *fname, unsigned long fname_len);
UIWIDGET_SCILAB_IMPEXP int sci_uiset(char *fname, unsigned long fname_len);
int sci_createUIWidgetHandle(char *fname, unsigned long fname_len);

/*--------------------------------------------------------------------------*/
#endif /* __GW_UIWIDGET_H__ */
/*--------------------------------------------------------------------------*/