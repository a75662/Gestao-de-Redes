#include <PRIM_set_request.h>

RES buildPDU_setReq(VarBindList_t* varlist, long reqID, char* cs, char* v) {
	SetRequest_PDU_t* setRequestPDU;
	setRequestPDU = calloc(1, sizeof(SetRequest_PDU_t));
	setRequestPDU->request_id = reqID;
	setRequestPDU->error_index = 0;
	setRequestPDU->error_status = 0;
	setRequestPDU->variable_bindings = *varlist;

	PDUs_t *pdu;
	pdu = calloc(1, sizeof(PDUs_t));
	pdu->present = PDUs_PR_set_request;
	pdu->choice.set_request = *setRequestPDU;

	size_t buf_size = 1024;
	uint8_t* buf = calloc(buf_size, sizeof(uint8_t));
	asn_enc_rval_t ret =
		asn_encode_to_buffer(0, ATS_BER, &asn_DEF_PDUs, pdu, buf, buf_size);

	return buildMsg(buf, ret, cs, atol(v));
}

VarBind_t* varBinding_setReq(ObjectSyntax_t* syntax, ObjectName_t* name) {
	VarBind_t* var_bind;
	var_bind = calloc(1, sizeof(VarBind_t));
	var_bind->name = *name;
	var_bind->choice.present = choice_PR_value;
	var_bind->choice.choice.value = *syntax;

	return var_bind;
}

VarBind_t* varsObject(SimpleSyntax_t* simple, ApplicationSyntax_t* app, char* oid_str) {
	ObjectSyntax_t* object_syntax;
	object_syntax = calloc(1, sizeof(ObjectSyntax_t));
	if (simple != NULL) {
		object_syntax->present = ObjectSyntax_PR_simple;
		object_syntax->choice.simple = *simple;
	}
	else {
		object_syntax->present = ObjectSyntax_PR_application_wide;
		object_syntax->choice.application_wide = *app;
	}

	ObjectName_t* object_name;
	object_name = calloc(1, sizeof(ObjectName_t));
	//=============================================
	size_t oid_size = 16;
	uint8_t* oid = calloc(oid_size, sizeof(uint8_t));
	int i = 0;
	char* token = strtok(oid_str, ".");
	do {
		oid[i++] = atoi(token);
		token = strtok(NULL, ".");
	}
	while (token != NULL);
	//=============================================
	object_name->buf = oid;
	object_name->size = i;
	return varBinding_setReq(object_syntax, object_name);
}

VarBind_t* simple_setRequest(char* oid, char* type, char* val){
	SimpleSyntax_t* simple;
	simple = calloc(1, sizeof(SimpleSyntax_t));

	if (!strcmp(type, "integer")) {
		simple->present = SimpleSyntax_PR_integer_value;
		simple->choice.integer_value = atol(val);
		return varsObject(simple, NULL, oid);
	}
	else if (!strcmp(type, "string")) {
		simple->present = SimpleSyntax_PR_string_value;
		OCTET_STRING_t* s = calloc(1, sizeof(OCTET_STRING_t));
		if (!OCTET_STRING_fromString(s, val))
			simple->choice.string_value = *s;
		return varsObject(simple, NULL, oid);
	}
	else if (!strcmp(type, "objectID")) {
		simple->present = SimpleSyntax_PR_objectID_value;
		OBJECT_IDENTIFIER_t* obj = calloc(1, sizeof(OBJECT_IDENTIFIER_t));
		//=============================================
		size_t oid_size = 16;
		uint8_t* oid_buf = calloc(oid_size, sizeof(uint8_t));
		int i = 0;
		char* token = strtok(val, ".");
		do {
			oid_buf[i++] = atoi(token);
			token = strtok(NULL, ".");
		}
		while (token != NULL);
		//=============================================
		obj->buf = oid_buf;
		obj->size = i;
		simple->choice.objectID_value = *obj;
		return varsObject(simple, NULL, oid);
	}
	else return app_setRequest(type, val, oid);
}

VarBind_t* app_setRequest(char* oid, char* type, char* val){
	ApplicationSyntax_t* app;
	app = calloc(1, sizeof(ApplicationSyntax_t));

	if (!strcmp(type, "IpAddress")) {
		app->present = ApplicationSyntax_PR_ipAddress_value;
		OCTET_STRING_t* ip = calloc(1, sizeof(OCTET_STRING_t));
		OCTET_STRING_fromBuf(ip, val, -1);
		app->choice.ipAddress_value = (IpAddress_t) *ip;
		return varsObject(NULL, app, oid);
	}
	if (!strcmp(type, "Counter32")) {
		app->present = ApplicationSyntax_PR_counter_value;
		app->choice.counter_value = atol(val);
		return varsObject(NULL, app, oid);
	}
	if (!strcmp(type, "TimeTicks")) {
		app->present = ApplicationSyntax_PR_timeticks_value;
		app->choice.timeticks_value = atol(val);
		return varsObject(NULL, app, oid);
	}
	if (!strcmp(type, "Opaque")) {
		app->present = ApplicationSyntax_PR_arbitrary_value;
		OCTET_STRING_t* s = calloc(1, sizeof(OCTET_STRING_t));
		if (!OCTET_STRING_fromString(s, val))
			app->choice.arbitrary_value = *s;
		return varsObject(NULL, app, oid);
	}
	if (!strcmp(type, "Counter64")) {
		app->present = ApplicationSyntax_PR_big_counter_value;
		INTEGER_t* num = calloc(1, sizeof(INTEGER_t));
		asn_ulong2INTEGER(num, atol(val));
		app->choice.big_counter_value = *num;
		return varsObject(NULL, app, oid);
	}
	if (!strcmp(type, "Unsigned32")) {
		app->present = ApplicationSyntax_PR_unsigned_integer_value;
		app->choice.unsigned_integer_value = atol(val);
		return varsObject(NULL, app, oid);
	}
}

/**
* reqID: indice do request, definido pela aplicacao
* cs: community string
* v: versao
* tail: array de strings com o seguinte formato: [oid, type, value, oid, type, value, ...]
* size: tamanho do array tail
* RETORNA um RES que contem a codificacao e o numero de bytes respetivo
*/
RES setReqHandler(long reqID, char* cs, char* v, char** tail, int size) {
	VarBindList_t* varlist;
	varlist = calloc(1, sizeof(VarBindList_t));

	int i;
	for (i = 0; i < size; i+=3)	 {
		VarBind_t* vb = simple_setRequest(tail[i], tail[i+1], tail[i+2]);
		ASN_SEQUENCE_ADD(&varlist->list, vb);
	}

	return buildPDU_setReq(varlist, reqID, cs, v);
}
