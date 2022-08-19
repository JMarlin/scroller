#ifndef WEBOBJECT_H
#define WEBOBJECT_H

#include "bufloader.h"

#define _O (WebDataNode*)&(WebDataObjectNode){.type=Object,.data=(WebDataNodeKvp[]){
#define O_ ,(WebDataNodeKvp){0}}}
#define _L (WebDataNode*)&(WebDataListNode){.type=List,.data=(WebDataNode*[]){
#define L_ ,(WebDataNode*){0}}}
#define N(x) (WebDataNode*)&(WebDataNumberNode){ Number, (x) }

typedef enum WebDataNodeType_E {
    End = 0, Object = 1, List = 2, String = 3, Bool = 4, Number = 5
} WebDataNodeType;

typedef struct WebDataNode_S {
    WebDataNodeType type;
} WebDataNode;

typedef struct WebDataNodeKvp_S {
    char* key;
    WebDataNode* value;
} WebDataNodeKvp;

typedef struct WebDataObjectNode_S {
    WebDataNodeType type;
    WebDataNodeKvp* data;
} WebDataObjectNode;

typedef struct WebDataListNode_S {
    WebDataNodeType type;
    WebDataNode** data;
} WebDataListNode;

typedef struct WebDataStringNode_S {
    WebDataNodeType type;
    char* data;
} WebDataStringNode;

typedef struct WebDataBoolNode_S {
    WebDataNodeType type;
    int data;
} WebDataBoolNode;

typedef struct WebDataNumberNode_S {
    WebDataNodeType type;
    int data;
} WebDataNumberNode;

WebDataObjectNode WebDataObjectNode_new(WebDataNodeKvp kvps[]);
int WebStringData_serializedLength(char* data);
int WebObjectData_serializedLength(WebDataObjectNode* webObject);
int WebData_serializedLength(WebDataNode* webData);
int WebStringData_serializedLength(char* data);
char* WebData_serializeStringToBuffer(char* data, char* buf);
char* WebData_serializeBoolToBuffer(WebDataBoolNode* webBool, char* buf);
char* WebData_serializeNumberToBuffer(WebDataNumberNode* webNumber, char* buf);
char* WebData_serializeObjectToBuffer(WebDataObjectNode* webObject, char* buf);
char* WebData_serializeListToBuffer(WebDataListNode* webList, char* buf);
char* WebData_serializeAsJSONToBuffer(WebDataNode* webData, char* buf);
char* WebData_serializeToJSON(WebDataNode* webData);
char* WebData_parseStringData(BufLoader* bufLoader);
WebDataNode* WebData_parseJSONList(BufLoader* bufLoader);
WebDataNode* WebData_parseJSONString(BufLoader* bufLoader);
WebDataNode* WebData_parseJSONBoolean(BufLoader* bufLoader);
WebDataNode* WebData_parseJSONNumber(BufLoader* bufLoader);
WebDataNode* WebData_parseFromJSON(BufLoader* bufLoader);
void WebData_free(WebDataNode* webData);
void WebData_consumeAsJSONResponse(WebDataNode* webData, int socket);

#endif //WEBOBJECT_H