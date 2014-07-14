/***************************************************************************
            kdbtools.c  -  Elektra High Level Methods
                             -------------------
    begin                : Sat Jan 22 2005
    copyright            : (C) 2005 by Avi Alkalay
    email                : avi@unix.sh
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/




/* Subversion stuff

$Id$

*/

#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <libxml/xmlreader.h>
#include <libxml/xmlschemas.h>

#include <kdbtools.h>
#include <kdbinternal.h>

/*
 * Processes the current <key> node from reader, converting from XML
 * to a Key object, and ksAppendKey() it to ks.
 *
 * See keyToStream() for an example of a <key> node.
 *
 * This function is completelly dependent on libxml.
 * 
 * @param ks where to put the resulting reded key
 * @param context a prent key name, so a full name can be calculated
 *        if the XML node for the current key only provides a basename
 * @param reader where to read from
 */
static int consumeKeyNode(KeySet *ks, const char *context, xmlTextReaderPtr reader)
{
	xmlChar *nodeName=0;
	xmlChar *keyNodeName=0;
	xmlChar *buffer=0;
	xmlChar *privateContext=0;
	Key *newKey=0;
	int appended=0;

	/* printf("%s", KDB_SCHEMA_PATH); */
	
	keyNodeName=xmlTextReaderName(reader);
	if (!strcmp((char *)keyNodeName,"key")) {
		mode_t isdir=0;
		int isbin=0;
		int end=0;
		
		newKey=keyNew(0);

		/* a <key> must have one of the following:
		   - a "name" attribute, used as an absolute name overriding the context
		   - a "basename" attribute, that will be appended to the current context
		   - a "parent" plus "basename" attributes, both appended to current context
		   - only a "parent", appended to current context
		*/
		buffer=xmlTextReaderGetAttribute(reader,(const xmlChar *)"name");
		if (buffer) {
			/* set absolute name */
			keySetName(newKey,(char *)buffer);
			xmlFree(buffer); buffer=0;
		} else {
			/* logic for relative name calculation */
			
			privateContext=xmlTextReaderGetAttribute(reader,
				(const xmlChar *)"parent");
			buffer=xmlTextReaderGetAttribute(reader,
				(const xmlChar *)"basename");

			if (context) keySetName(newKey,context);
			if (privateContext) keyAddBaseName(newKey, (char *)privateContext);
			if (buffer) keyAddBaseName(newKey,(char *)buffer);

			xmlFree(privateContext); privateContext=0;
			xmlFree(buffer); buffer=0;
		}


		/* test for a short value attribute, instead of <value> bellow */
		buffer=xmlTextReaderGetAttribute(reader,(const xmlChar *)"value");
		if (buffer) {
			keySetRaw(newKey,buffer,elektraStrLen((char *)buffer));
			xmlFree(buffer); buffer=0;
		}

		/* Parse UID */
		buffer=xmlTextReaderGetAttribute(reader,(const xmlChar *)"uid");
		if (buffer) {
			int errsave = errno;
			char * endptr;
			long int uid = strtol ((const char *)buffer, &endptr, 10);
			errno = errsave;
			if (endptr != '\0' && *endptr == '\0')
			{
				keySetUID(newKey,uid);
			}
			xmlFree(buffer); buffer=0;
		}

		/* Parse GID */
		buffer=xmlTextReaderGetAttribute(reader,(const xmlChar *)"gid");
		if (buffer) {
			int errsave = errno;
			char * endptr;
			long int gid = strtol ((const char *)buffer, &endptr, 10);
			errno = errsave;
			if (endptr != '\0' && *endptr == '\0')
			{
				keySetGID(newKey,gid);
			}
			xmlFree(buffer); buffer=0;
		}

		/* Parse mode permissions */
		buffer=xmlTextReaderGetAttribute(reader,(const xmlChar *)"mode");
		int errsave = errno;
		if (buffer) keySetMode(newKey,strtol((char *)buffer,0,0));
		errno = errsave;
		xmlFree(buffer);



		if (xmlTextReaderIsEmptyElement(reader)) {
			/* we have a <key ..../> element */
			if (newKey && !appended) {
				ksAppendKey(ks,newKey);
				appended=1;
				end=1;
			}
		}

		buffer=xmlTextReaderGetAttribute(reader,(const xmlChar *)"type");
		if (buffer)
		{
			if (!strcmp((char *)buffer,"binary")) isbin = 1;
			else if (!strcmp((char *)buffer,"bin")) isbin = 1;
		}
		xmlFree(buffer);

		/* If "isdir" appears, everything different from "0", "false" or "no"
		marks it as a dir key */
		buffer=xmlTextReaderGetAttribute(reader,(const xmlChar *)"isdir");
		if (!isdir && buffer)
		{
			if (	strcmp((char *)buffer,"0") &&
				strcmp((char *)buffer,"false") &&
				strcmp((char *)buffer,"no"))
				isdir = 1;
			else	isdir = 0;
		}
		xmlFree(buffer);

		if (isdir) keySetDir(newKey);
		if (isbin) keySetMeta (newKey, "binary", "");

		/* Parse everything else */
		while (!end) {
			xmlTextReaderRead(reader);
			nodeName=xmlTextReaderName(reader);

			if (!strcmp((char *)nodeName,"value")) {
				if (xmlTextReaderIsEmptyElement(reader) ||
					xmlTextReaderNodeType(reader)==15)
				{
					xmlFree (nodeName);
					continue;
				}
					
				xmlTextReaderRead(reader);
				buffer=xmlTextReaderValue(reader);
				
				if (buffer) {
					/* Key's value type was already set above */
					if (keyIsBinary(newKey)) {
						/* TODO binary values
						char *unencoded=0;
						size_t unencodedSize;
						
						unencodedSize=elektraStrLen((char *)buffer)/2;
						unencoded=malloc(unencodedSize);
						unencodedSize=kdbbDecode((char *)buffer,unencoded);
						if (!unencodedSize) return -1;
							keySetRaw(newKey,unencoded,unencodedSize);
						free(unencoded);
						*/
					} else keySetRaw(newKey,buffer,elektraStrLen((char *)buffer));
				}
				xmlFree(buffer);
			} else if (!strcmp((char *)nodeName,"comment")) {
				ssize_t commentSize=0;
				
				if (xmlTextReaderIsEmptyElement(reader) ||
					xmlTextReaderNodeType(reader)==15)
				{
					xmlFree (nodeName);
					continue;
				}
					
				xmlTextReaderRead(reader);
				buffer=xmlTextReaderValue(reader);
				
				if ((commentSize=keyGetCommentSize(newKey)) > 1) {
					/*Multiple line comment*/
					char *tmpComment=0;
					tmpComment=malloc(commentSize+
						xmlStrlen(buffer)*sizeof(xmlChar)+1);

					if (tmpComment) {
						keyGetComment(newKey,tmpComment,commentSize);

						strcat(tmpComment,"\n");
						strcat(tmpComment,(char *)buffer);

						keySetComment(newKey,tmpComment);

						free(tmpComment); tmpComment=0;
					}
				} else keySetComment(newKey,(char *)buffer);
				xmlFree(buffer);
			} else if (!strcmp((char *)nodeName,"key")) {
				/* Here we found </key> or a sub <key>.
				   So include current key in the KeySet. */
				if (newKey && !appended) {
					ksAppendKey(ks,newKey);
					appended=1;
				}
				
				if (xmlTextReaderNodeType(reader)==15)
					/* found a </key> */
					end=1;
				else {
					/* found a sub <key> */
					if (! keyIsDir(newKey)) {
						keySetDir(newKey);
					}
					/* prepare the context (parent) */
					consumeKeyNode(ks,newKey->key,reader);
				}
			}

			xmlFree (nodeName);
		}

		if (privateContext) xmlFree(privateContext);

		/* seems like we forgot the key, lets delete it */
		if (newKey && !appended) {
			keyDel (newKey);
			appended=1;
		}
	}

	xmlFree(keyNodeName);
	
	return 0;
}




static int consumeKeySetNode(KeySet *ks, const char *context, xmlTextReaderPtr reader)
{
	xmlChar *nodeName=0;
	xmlChar *keySetNodeName=0;
	xmlChar *privateContext=0;
	xmlChar fullContext[800]="";
	
	keySetNodeName=xmlTextReaderName(reader);
	if (!strcmp((char *)keySetNodeName,"keyset")) {
		int end=0;

		privateContext=xmlTextReaderGetAttribute(reader,(const xmlChar *)"parent");
		if (context && privateContext) {
			xmlStrPrintf(fullContext,sizeof(fullContext),
				(const xmlChar *)"%s/%s", context, privateContext);
		}

		/* Parse everything else */
		while (!end) {
			xmlTextReaderRead(reader);
			nodeName=xmlTextReaderName(reader);

			if (!strcmp((char *)nodeName,"key")) {
				if (privateContext) consumeKeyNode(ks,(char *)(*fullContext?fullContext:privateContext),reader);
				else consumeKeyNode(ks,context,reader);
			} else if (!strcmp((char *)nodeName,"keyset")) {
				/* A <keyset> can have nested <keyset>s */
				if (xmlTextReaderNodeType(reader)==15)
					/* found a </keyset> */
					end=1;
				else if (privateContext)
					consumeKeySetNode(ks, (char *)(*fullContext?fullContext:privateContext), reader);
				else consumeKeySetNode(ks, context, reader);
			}
			xmlFree(nodeName);
		}
		if (privateContext) xmlFree(privateContext),privateContext=0;
	}
	xmlFree (keySetNodeName);
	return 0;
}



/*
 * This is the workhorse behind for ksFromXML() and ksFromXMLfile().
 * It will process the entire XML document in reader and convert and
 * save it in ks KeySet. Each node is processed by the processNode() function.
 *
 * This function is completely dependent on libxml.
 */
static int ksFromXMLReader(KeySet *ks,xmlTextReaderPtr reader)
{
	int ret = 0;
	xmlChar *nodeName=0;

	ret = xmlTextReaderRead(reader); /* go to first node */
	while ((ret == 1)) {
		/* walk node per node until the end of the stream */
		nodeName=xmlTextReaderName(reader);
		
		if (!strcmp((char *)nodeName,"key"))
			consumeKeyNode(ks, 0, reader);
		else if (!strcmp((char *)nodeName,"keyset"))
			consumeKeySetNode(ks, 0, reader);
		
		ret = xmlTextReaderRead(reader);

		xmlFree (nodeName);
	}

	return ret;
}

/*
static int isValidXML(xmlDocPtr doc,char *schemaPath)
{
	xmlSchemaPtr wxschemas = NULL;
	xmlSchemaValidCtxtPtr ctxt;
	xmlSchemaParserCtxtPtr ctxt2=NULL;
	int ret=0;

	ctxt2 = xmlSchemaNewParserCtxt(schemaPath);


	if (ctxt2==NULL) {
		xmlFreeDoc(doc);
		return 1;
	}
	
	xmlSchemaSetParserErrors(ctxt2,
		(xmlSchemaValidityErrorFunc) fprintf,
		(xmlSchemaValidityWarningFunc) fprintf,
		stderr);
	wxschemas = xmlSchemaParse(ctxt2);
	
	if (wxschemas==NULL) {
		xmlSchemaFreeParserCtxt(ctxt2);
		xmlFreeDoc(doc);
		return 1;
	}
	
	ctxt = xmlSchemaNewValidCtxt(wxschemas);
	xmlSchemaSetValidErrors(ctxt,
		(xmlSchemaValidityErrorFunc) fprintf,
		(xmlSchemaValidityWarningFunc) fprintf,
		stderr);
	
	if (ctxt==NULL) {
		xmlSchemaFree(wxschemas);
		xmlSchemaFreeParserCtxt(ctxt2);
		xmlFreeDoc(doc);
		return 1;
	}
	
	ret = xmlSchemaValidateDoc(ctxt, doc);
	xmlSchemaFreeValidCtxt(ctxt);
	xmlSchemaFree(wxschemas);
	xmlSchemaFreeParserCtxt(ctxt2);

	return ret;
}
*/



/**
 * Given an XML @p filename, open it, validate schema, process nodes,
 * convert and save it in the @p ks KeySet.
 *
 * Currently, the XML file can have many root @c @<keyset@> and @c @<key@> nodes.
 * They will all be reduced to simple keys returned in @p ks.
 *
 * To check if the xml file is valid (best before you read from it):
 * @code
#include 
char schemaPath[513];
schemaPath[0]=0;
ret=kdbGetString(handle, KDB_SCHEMA_PATH_KEY,schemaPath,sizeof(schemaPath));

if (ret==0) ret = isValidXML(filename,schemaPath);
else ret = isValidXML(filename,KDB_SCHEMA_PATH); 
 * @endcode
 *
 * @retval -1 on error
 * @retval 0 if file could not be opened
 * @param ks the keyset
 * @param filename the file to parse
 * @ingroup stream
 */
int ksFromXMLfile(KeySet *ks, const char *filename)
{
	xmlTextReaderPtr reader = 0;
	xmlDocPtr doc = 0;
	int ret=0;

	doc = xmlParseFile(filename);
	if (doc==0)
	{
		// TODO: distinguish between parser errors and no file
		xmlCleanupParser();
		return 0;
	}

	reader=xmlReaderWalker(doc);
	if (reader)
	{
		ret=ksFromXMLReader(ks,reader);
		xmlFreeTextReader (reader);
	}
	else {
		ret = -1;
	}

	xmlFreeDoc(doc);

	xmlCleanupParser();
	return ret;
}





/**
 * Given a file descriptor (that can be @p stdin) for an XML file, validate
 * schema, process nodes, convert and save it in the @p ks KeySet.
 *
 * @param ks keyset
 * @param fd POSIX file descriptior
 * @ingroup stream
 */
int ksFromXML(KeySet *ks, int fd)
{
	// a complete XML document is expected
	xmlTextReaderPtr reader=0;
	int ret;
	reader=xmlReaderForFd(fd,"file:/tmp/imp.xml",0,0);
	if (reader) {
		ret=ksFromXMLReader(ks,reader);
	} else {
		printf("kdb: Unable to open file descriptor %d for XML reading\n", fd);
		return 1;
	}
	return ret;
}

