/* 
   Copyright (C) Cfengine AS

   This file is part of Cfengine 3 - written and maintained by Cfengine AS.
 
   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; version 3.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License  
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

  To the extent this program is licensed as part of the Enterprise
  versions of Cfengine, the applicable Commerical Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.

*/

#include "verify_measurements.h"

#include "promises.h"
#include "files_names.h"
#include "attributes.h"
#include "cfstream.h"
#include "logging.h"
#ifdef HAVE_NOVA
#include "history.h"
#endif

#ifndef HAVE_NOVA
static void VerifyMeasurement(double *this, Attributes a, Promise *pp);
#endif

static int CheckMeasureSanity(Attributes a, Promise *pp);

/*****************************************************************************/

void VerifyMeasurementPromise(double *this, Promise *pp)
{
    Attributes a = { {0} };

    if (pp->done)
    {
        if (pp->ref)
        {
            CfOut(OUTPUT_LEVEL_VERBOSE, "", "Skipping static observation %s (%s), already done", pp->promiser, pp->ref);
        }
        else
        {
            CfOut(OUTPUT_LEVEL_VERBOSE, "", "Skipping static observation %s, already done", pp->promiser);
        }

        return;
    }

    PromiseBanner(pp);

    a = GetMeasurementAttributes(pp);

    if (!CheckMeasureSanity(a, pp))
    {
        return;
    }

    VerifyMeasurement(this, a, pp);
}

/*****************************************************************************/

static int CheckMeasureSanity(Attributes a, Promise *pp)
{
    int retval = true;

    if (!IsAbsPath(pp->promiser))
    {
        cfPS(OUTPUT_LEVEL_ERROR, CF_INTERPT, "", pp, a, "The promiser \"%s\" of a measurement was not an absolute path",
             pp->promiser);
        PromiseRef(OUTPUT_LEVEL_ERROR, pp);
        retval = false;
    }

    if (a.measure.data_type == DATA_TYPE_NONE)
    {
        cfPS(OUTPUT_LEVEL_ERROR, CF_INTERPT, "", pp, a, "The promiser \"%s\" did not specify a data type\n", pp->promiser);
        PromiseRef(OUTPUT_LEVEL_ERROR, pp);
        retval = false;
    }
    else
    {
        if ((a.measure.history_type) && (strcmp(a.measure.history_type, "weekly") == 0))
        {
            switch (a.measure.data_type)
            {
            case DATA_TYPE_COUNTER:
            case DATA_TYPE_STRING:
            case DATA_TYPE_INT:
            case DATA_TYPE_REAL:
                break;

            default:
                cfPS(OUTPUT_LEVEL_ERROR, CF_INTERPT, "", pp, a,
                     "The promiser \"%s\" cannot have history type weekly as it is not a number\n", pp->promiser);
                PromiseRef(OUTPUT_LEVEL_ERROR, pp);
                retval = false;
                break;
            }
        }
    }

    if ((a.measure.select_line_matching) && (a.measure.select_line_number != CF_NOINT))
    {
        cfPS(OUTPUT_LEVEL_ERROR, CF_INTERPT, "", pp, a,
             "The promiser \"%s\" cannot select both a line by pattern and by number\n", pp->promiser);
        PromiseRef(OUTPUT_LEVEL_ERROR, pp);
        retval = false;
    }

    if (!a.measure.extraction_regex)
    {
        CfOut(OUTPUT_LEVEL_VERBOSE, "", "No extraction regex, so assuming whole line is the value");
    }
    else
    {
        if ((!strchr(a.measure.extraction_regex, '(')) && (!strchr(a.measure.extraction_regex, ')')))
        {
            cfPS(OUTPUT_LEVEL_ERROR, CF_INTERPT, "", pp, a,
                 "The extraction_regex must contain a single backreference for the extraction\n");
            retval = false;
        }
    }

    return retval;
}

#ifndef HAVE_NOVA
static void VerifyMeasurement(double *this, Attributes a, Promise *pp)
{
}
#endif
