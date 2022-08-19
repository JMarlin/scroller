#include "webobject.h"
#include "http.h"
#include "util.h"
#include <string.h>

int WebObjectData_elementCount(WebDataObjectNode* webObject) {
    int count = 0;
    for(; webObject->data[count].value; count++) {}
    return count;
}

int WebObjectData_serializedLength(WebDataObjectNode* webObject) {
    int elemCount = WebObjectData_elementCount(webObject);
    int commaAndColonCount = elemCount == 0 ? 0 : (2 * elemCount - 1);
    int totalChars = commaAndColonCount + 2; //Brackets

    for(int i = 0; i < elemCount; i++) 
        totalChars +=
            WebStringData_serializedLength(webObject->data[i].key)
          + WebData_serializedLength(webObject->data[i].value);

    return totalChars;
}

int WebListData_elementCount(WebDataListNode* webList) {
    int count = 0;
    for(; webList->data[count]; count++) {}
    return count;
}

int WebListData_serializedLength(WebDataListNode* webList) {
    int elemCount = WebListData_elementCount(webList);
    int commaCount = elemCount == 0 ? 0 : elemCount - 1;
    int totalChars = commaCount + 2; //Brackets

    for(int i = 0; i < elemCount; i++) 
        totalChars += WebData_serializedLength(webList->data[i]);

    return totalChars;
}

int WebData_serializedLength(WebDataNode* webData) {
    if(webData->type == Object) return WebObjectData_serializedLength((WebDataObjectNode*)webData);
    if(webData->type == List)   return WebListData_serializedLength((WebDataListNode*)webData);
    if(webData->type == String) return WebStringData_serializedLength(((WebDataStringNode*)webData)->data);
    if(webData->type == Bool)   return ((WebDataBoolNode*)webData)->data ? 4 : 5;
    if(webData->type == Number) return decimalDigits(((WebDataNumberNode*)webData)->data);

    return 0;
}

int WebStringData_serializedLength(char* data) {
    return strlen(data) + 2; //TODO: Handle escape characters
}

//TODO: Handle escape characters
char* WebData_serializeStringToBuffer(char* data, char* buf) {
    *(buf++) = '"';
    buf = bufferInsertString(data, buf);
    *(buf++) = '"';

    return buf;
}

char* WebData_serializeBoolToBuffer(WebDataBoolNode* webBool, char* buf) {
    char* text = webBool->data ? "true" : "false";

    return bufferInsertString(text, buf);
}

char* WebData_serializeNumberToBuffer(WebDataNumberNode* webNumber, char* buf) {
    char stringText[32];
    intToStrN(webNumber->data, stringText, 32);

    return bufferInsertString(stringText, buf);
}

char* WebData_serializeObjectToBuffer(WebDataObjectNode* webObject, char* buf) {
    *(buf++) = '{';

    int elemCount = WebObjectData_elementCount(webObject);

    for(int i = 0; i < elemCount; i++) {
        if(i > 0) *(buf++) = ',';
        buf = WebData_serializeStringToBuffer(webObject->data[i].key, buf);
        *(buf++) = ':';
        buf = WebData_serializeAsJSONToBuffer(webObject->data[i].value, buf);
    }
    
    *(buf++) = '}';

    return buf;
}

char* WebData_serializeListToBuffer(WebDataListNode* webList, char* buf) {
    *(buf++) = '[';

    int elemCount = WebListData_elementCount(webList);

    for(int i = 0; i < elemCount; i++) {
        if(i > 0) *(buf++) = ',';
        buf = WebData_serializeAsJSONToBuffer(webList->data[i], buf);
    }
    
    *(buf++) = ']';

    return buf;
}

char* WebData_serializeAsJSONToBuffer(WebDataNode* webData, char* buf) {
    if(webData->type == Object) return WebData_serializeObjectToBuffer((WebDataObjectNode*)webData, buf);
    if(webData->type ==List)    return WebData_serializeListToBuffer((WebDataListNode*)webData, buf);
    if(webData->type ==String)  return WebData_serializeStringToBuffer(((WebDataStringNode*)webData)->data, buf);
    if(webData->type ==Bool)    return WebData_serializeBoolToBuffer((WebDataBoolNode*)webData, buf);
    if(webData->type ==Number)  return WebData_serializeNumberToBuffer((WebDataNumberNode*)webData, buf);

    return 0;
}

char* WebData_serializeToJSON(WebDataNode* webData) {
    int length = WebData_serializedLength(webData);
    char* outBuffer = (char*)malloc(length + 1);

    if(!outBuffer) return outBuffer;
    
    WebData_serializeAsJSONToBuffer(webData, outBuffer);

    outBuffer[length] = 0;

    return outBuffer;
}

char* WebData_parseStringData(BufLoader* bufLoader){
#warning WebData_parseStringData not implemented
    return "";
}

WebDataNode* WebData_parseJSONObject(BufLoader* bufLoader) {
    int kvpCount = 0;
    ExpandoBuf kvps = ExpandoBuf_new(sizeof(WebDataNodeKvp));

    if(BufLoader_peek(bufLoader) != '{') return 0;

    BufLoader_next(bufLoader);

    while(1) {
        BufLoader_skipWhitespace(bufLoader);
        char* key = WebData_parseStringData(bufLoader);

        if(!key) {
            //TODO: Actually, we need to free all of the PREVIOUSLY allocated keys and values as well
            free(kvps.data);

            return 0;
        }

        BufLoader_skipWhitespace(bufLoader);

        if(BufLoader_peek(bufLoader) != ':') {
            free(kvps.data);

            return 0;
        }

        WebDataNode* value = WebData_parseFromJSON(bufLoader);

        if(!value) {
            //TODO: Actually, we need to free all of the PREVIOUSLY allocated keys and values as well
            free(kvps.data);
            free(key);

            return 0;
        }

        //TODO: What do we do if expansion doesn't work and we lose refs to all of the keys and values we need to free?
        kvps = ExpandoBuf_ExpandTo(kvps, kvpCount + 1);
        ((WebDataNodeKvp*)kvps.data)[kvpCount].key = key;
        ((WebDataNodeKvp*)kvps.data)[kvpCount].value = value;
        kvpCount++;

        BufLoader_skipWhitespace(bufLoader);

        if(BufLoader_peek(bufLoader) == '}') {
            break;
        }
    }

    WebDataObjectNode* object = (WebDataObjectNode*)malloc(sizeof(WebDataObjectNode));

    if(!object) {
        //TODO: Actually, we need to free all of the PREVIOUSLY allocated keys and values as well
        free(kvps.data);

        return (WebDataNode*)object;
    }

    object->data = (WebDataNodeKvp*)kvps.data;

    return (WebDataNode*)object;
}

WebDataNode* WebData_parseJSONList(BufLoader* bufLoader) {
#warning WebData_parseJSONList not implemented
    return (WebDataNode*)0;
}

WebDataNode* WebData_parseJSONString(BufLoader* bufLoader) {
#warning WebData_parseJSONString not implemented
    return (WebDataNode*)0;
}

WebDataNode* WebData_parseJSONBoolean(BufLoader* bufLoader) {
#warning WebData_parseJSONBoolean not implemented
    return (WebDataNode*)0;
}

WebDataNode* WebData_parseJSONNumber(BufLoader* bufLoader) {
#warning WebData_parseJSONNumber not implemented
    return (WebDataNode*)0;
}

WebDataNode* WebData_parseFromJSON(BufLoader* bufLoader) {
    WebDataNode* resultNode = 0;

    BufLoader_skipWhitespace(bufLoader);

    if((resultNode = WebData_parseJSONObject(bufLoader))  != 0) return resultNode;
    if((resultNode = WebData_parseJSONList(bufLoader))    != 0) return resultNode;
    if((resultNode = WebData_parseJSONString(bufLoader))  != 0) return resultNode;
    if((resultNode = WebData_parseJSONBoolean(bufLoader)) != 0) return resultNode;
    if((resultNode = WebData_parseJSONNumber(bufLoader))  != 0) return resultNode;

    return resultNode;
}

void WebData_free(WebDataNode* webData) {
    int elemCount; 

    switch(webData->type) {
        Object:
            elemCount = WebObjectData_elementCount((WebDataObjectNode*)webData);
            for(int i = 0; i < elemCount; i++)
                WebData_free(((WebDataObjectNode*)webData)->data[i].value);
            free(webData);
            break;
        List:
            elemCount = WebListData_elementCount((WebDataListNode*)webData);
            for(int i = 0; i < elemCount; i++)
                WebData_free(((WebDataListNode*)webData)->data[i]);
            free(webData);
            break;
        default:
            break;
    }
    //NOTE: non-dynamically sized nodes are always either created on the stack or otherwise managed by the object creator
}

//TODO: Probably move this to some http-json-glue module
void WebData_consumeAsJSONResponse(WebDataNode* webData, int socket) {
    char contentLengthStr[32];
    int length = WebData_serializedLength(webData);
    char* bodyText = WebData_serializeToJSON(webData); 

    WebData_free(webData);
    calcContentLengthStrN(bodyText, contentLengthStr, 32);

    HttpResponse res = (HttpResponse){
        .protocol = "HTTP/1.1",
        .responseCode = 200,
        .responseDescription = "OK",
        .headerCount = 2,
        .headers = (HttpHeader[]){
            { "Content-Type", "application/json" },
            { "Content-Length",  contentLengthStr }
        },
        .body = bodyText
    };

    HttpResponse_send(socket, &res);
    free(bodyText);
}

void WebData_consumeAsJSONResponse(WebDataNode* webData, int socket); 
