#include <stdio.h>
#include "msscript.h"
#include "dictionary.h"

#define TEST_RESULT(hr,e) if (FAILED(hr)) goto e;

void foo(void)
{
	HRESULT hr;
	IScriptControl *pIScriptControl;
    hr = CoCreateInstance(&CLSID_ScriptControl, NULL, CLSCTX_ALL, &IID_IScriptControl, &pIScriptControl);
    if (FAILED(hr)) {
    	fprintf(stderr, "Create instance failed.\n");
    	return;
    }
    /* Should use OLESTR rather than BSTR. Terminator '\0' required. */
    hr = pIScriptControl->lpVtbl->putLanguage(pIScriptControl, L"VBScript");
    TEST_RESULT(hr, Exception);
    hr = pIScriptControl->lpVtbl->ExecuteStatement(pIScriptControl, L"MsgBox \"Hello, world!\"");
    TEST_RESULT(hr, Exception);
    pIScriptControl->lpVtbl->Release(pIScriptControl);
    return;
Exception:
	fprintf(stderr, "Error: %X\n", hr);
    pIScriptControl->lpVtbl->Release(pIScriptControl);
}

void bar(void)
{
	HRESULT hr;
    IDictionary *pIDictionary;
    hr = CoCreateInstance(&CLSID_Dictionary, NULL, CLSCTX_ALL, &IID_IDictionary, &pIDictionary);
    if (FAILED(hr)) {
    	fprintf(stderr, "Create instance failed.\n");
    	return;
    }
    VARIANT sKey = {.vt = VT_BSTR, .bstrVal = SysAllocString(L"aaa")};
    VARIANT iVal = {.vt = VT_I2, .iVal = 123};
    hr = pIDictionary->lpVtbl->Add(pIDictionary, &sKey, &iVal);
    TEST_RESULT(hr, Exception);
    LONG cnt;
    pIDictionary->lpVtbl->Count(pIDictionary, &cnt);
    TEST_RESULT(hr, Exception);
    printf("Count: %d\n", cnt); /* not rigorous, though */
    hr = pIDictionary->lpVtbl->getItem(pIDictionary, &sKey, &iVal);
    TEST_RESULT(hr, Exception);
    printf("iVal: %d\n", iVal.iVal);
    pIDictionary->lpVtbl->Release(pIDictionary);
    return;
Exception:
	fprintf(stderr, "Error: %X\n", hr);
    pIDictionary->lpVtbl->Release(pIDictionary);
}

int main(void)
{
    HRESULT hr;
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
    	fprintf(stderr, "CoInitialize failed.\n");
    	return 0;
    }
    /* ScriptControl test */
    foo();
    /* Dictionary text */
    bar();
    CoUninitialize();
}