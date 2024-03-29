#ifndef PRIM_response
#define PRIM_response

#include <PDUs.h>
#include <Message.h>
#include <encodeAPI.h>

RES buildPDU_response(VarBindList_t* varlist, long reqID, long ei, long es);
VarBind_t* varBinding_response(ObjectSyntax_t* syntax, ObjectName_t* name);

/**
* reqID: indice do request, definido pela aplicacao
* ei: indice do erro
* es: estado do erro
* tail: array de strings com o seguinte formato: [oid, type, value, oid, type, value, ...]
* size: tamanho do array tail
* RETORNA um RES que contem a codificacao e o numero de bytes respetivo
*/
RES responseHandler(long reqID, char* ei, char* es, char* argv[], int n);

VarBind_t* varsObject_response(SimpleSyntax_t* simple, ApplicationSyntax_t* app, char* oid_str);
VarBind_t* simple_response(char* oid, char* type, char* val);
VarBind_t* app_response(char* oid, char* type, char* val);
VarBind_t* other_response(char* oid, char* type, char* val);

#endif
