/*
 *  libfb - FreeBASIC's runtime library
 *	Copyright (C) 2004-2006 Andre V. T. Vicentini (av1ctor@yahoo.com.br) and
 *  the FreeBASIC development team.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  As a special exception, the copyright holders of this library give
 *  you permission to link this library with independent modules to
 *  produce an executable, regardless of the license terms of these
 *  independent modules, and to copy and distribute the resulting
 *  executable under terms of your choice, provided that you also meet,
 *  for each linked independent module, the terms and conditions of the
 *  license of that module. An independent module is a module which is
 *  not derived from or based on this library. If you modify this library,
 *  you may extend this exception to your version of the library, but
 *  you are not obligated to do so. If you do not wish to do so, delete
 *  this exception statement from your version.
 */

/*
 *	dev_lpt - LPTx device
 *
 * chng: jul/2005 written [mjs]
 *       jun/2006 factored common code [jeffmarshall]
 *								removed dependencies on hardcoded "LPT" and "PRN"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "fb.h"
#include "fb_rterr.h"

static 
FB_FILE * fb_DevLptFindDeviceByName( int iPort, char * filename, int no_redir )
{
  size_t i;
	/* Test if the printer is already open. */
	for( i=0;
			 i!=FB_MAX_FILES;
			 ++i )
	{
		FB_FILE *handle = fb_fileTB + i;
		if( handle->type == FB_FILE_TYPE_PRINTER )
		{
			if( no_redir==FALSE || handle->redirection_to==NULL )
			{
				DEV_LPT_INFO *pInfo = (DEV_LPT_INFO*) handle->opaque;
				if( pInfo )
				{
					if( iPort == 0 || iPort == pInfo->iPort )
					{
						if( strcmp(pInfo->pszDevice, filename)==0 ) {
								/* bugcheck */
								DBG_ASSERT( handle!=FB_HANDLE_PRINTER
												&& handle!=FB_HANDLE_PRINTER );
								return handle;
						}
					}
				}
			}
		}
	}
	return( NULL );
}

static
char * fb_DevLptMakeDeviceName( DEV_LPT_PROTOCOL *lpt_proto )
{
	if( lpt_proto )
	{
		char * p = calloc( strlen(lpt_proto->proto) + strlen(lpt_proto->name) + 3, 1 );
		strcpy( p, lpt_proto->proto );
		strcat( p, ":" );
		strcat( p, lpt_proto->name );
		return( p );
	}
	return( NULL );
}


static FB_FILE_HOOKS fb_hooks_dev_lpt = {
    NULL,
    fb_DevLptClose,
    NULL,
    NULL,
    NULL,
    NULL,
    fb_DevLptWrite,
    fb_DevLptWriteWstr,
    NULL,
    NULL,
    NULL,
    NULL
};

/*:::::*/
int fb_DevLptOpen( FB_FILE *handle, const char *filename, size_t filename_len )
{
    FB_FILE *redir_handle = NULL;
		FB_FILE *tmp_handle = NULL;
		DEV_LPT_PROTOCOL *lpt_proto;
    DEV_LPT_INFO *info;
    int res;

    if (!fb_DevLptParseProtocol( &lpt_proto, filename, filename_len , TRUE) )
		{
			if( lpt_proto )
				free( lpt_proto );
			return fb_ErrorSetNum( FB_RTERROR_ILLEGALFUNCTIONCALL );
		}

    FB_LOCK();

    /* Determine the port number and a normalized device name */
    info = (DEV_LPT_INFO*) calloc(1, sizeof(DEV_LPT_INFO));
    info->uiRefCount = 1;
		info->iPort = lpt_proto->iPort;
		info->pszDevice = fb_DevLptMakeDeviceName( lpt_proto );

    /* Test if the printer is already open. */
		if( tmp_handle = fb_DevLptFindDeviceByName( info->iPort, info->pszDevice, FALSE ) )
		{
      redir_handle = tmp_handle;
      info = (DEV_LPT_INFO*) tmp_handle->opaque;
      ++info->uiRefCount;
		}

    /* Open the printer if not opened already */
    if( info->hPrinter == NULL ) {
        res = fb_PrinterOpen( info->iPort, filename, &info->hPrinter );
    } else {
        res = fb_ErrorSetNum( FB_RTERROR_OK );
        if( FB_HANDLE_USED(redir_handle) ) {
            /* We only allow redirection between OPEN "LPT1:" and LPRINT */
            if( handle==FB_HANDLE_PRINTER ) {
                redir_handle->redirection_to = handle;
                handle->width = redir_handle->width;
                handle->line_length = redir_handle->line_length;
            } else {
                handle->redirection_to = redir_handle;
            }
        } else {
            handle->width = 80;
        }
    }

    if( res == FB_RTERROR_OK ) {
        handle->hooks = &fb_hooks_dev_lpt;
        handle->opaque = info;
				handle->type = FB_FILE_TYPE_PRINTER;
    } else {
        if( info->pszDevice )
            free( info->pszDevice );
        free( info );
    }

		if( lpt_proto )
			free( lpt_proto );

    FB_UNLOCK();

	return res;
}

int fb_DevPrinterSetWidth( const char *pszDevice, int width, int default_width )
{
		FB_FILE *tmp_handle = NULL;
    int cur = ((default_width==-1) ? 80 : default_width);
    char *pszDev;
		DEV_LPT_PROTOCOL *lpt_proto;

    if (!fb_DevLptParseProtocol( &lpt_proto, pszDevice, strlen(pszDevice), TRUE) )
		{
			if( lpt_proto )
				free( lpt_proto );
			return fb_ErrorSetNum( FB_RTERROR_ILLEGALFUNCTIONCALL );
		}

		pszDev = fb_DevLptMakeDeviceName( lpt_proto );

    /* Test all printers. */
		if( tmp_handle = fb_DevLptFindDeviceByName( lpt_proto->iPort, pszDev, TRUE ) )
		{
      if( width!=-1 )
          tmp_handle->width = width;
      cur = tmp_handle->width;
		}

		if( lpt_proto )
			free( lpt_proto );
    free(pszDev);

    return cur;
}

int fb_DevPrinterGetOffset( const char *pszDevice )
{
		FB_FILE *tmp_handle = NULL;
    int cur = 0;
    char *pszDev;
		DEV_LPT_PROTOCOL *lpt_proto;

    if (!fb_DevLptParseProtocol( &lpt_proto, pszDevice, strlen(pszDevice), TRUE) )
		{
			if( lpt_proto )
				free( lpt_proto );
			return fb_ErrorSetNum( FB_RTERROR_ILLEGALFUNCTIONCALL );
		}

		pszDev = fb_DevLptMakeDeviceName( lpt_proto );

    /* Test all printers. */
		if( tmp_handle = fb_DevLptFindDeviceByName( lpt_proto->iPort, pszDev, TRUE ) )
      cur = tmp_handle->line_length;

		if( lpt_proto )
			free( lpt_proto );
    free(pszDev);

    return cur;

}

