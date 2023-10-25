#ifndef _MSSCRIPT_H_
#define _MSSCRIPT_H_

#include <oaidl.h>

#define C_DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

C_DEFINE_GUID(CLSID_ScriptControl, 0x0E59F1D5, 0x1FBE, 0x11D0, 0x8F, 0xF2, 0x00, 0xA0, 0xD1, 0x00, 0x38, 0xBC);
C_DEFINE_GUID(IID_IScriptControl, 0x0E59F1D3, 0x1FBE, 0x11D0, 0x8F, 0xF2, 0x00, 0xA0, 0xD1, 0x00, 0x38, 0xBC);

typedef interface IScriptControl IScriptControl;

/* incomplete. */
typedef void *IScriptModuleCollection;
typedef void *IScriptError;
typedef void *IScriptProcedureCollection;

typedef enum {
    Initialized = 0,
    Connected = 1
} ScriptControlStates;

typedef struct IScriptControlVtbl {
    BEGIN_INTERFACE

    /*** IUnknown methods ***/
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IScriptControl* This,
        REFIID riid,
        void **ppvObject);

    ULONG (STDMETHODCALLTYPE *AddRef)(
        IScriptControl* This);

    ULONG (STDMETHODCALLTYPE *Release)(
        IScriptControl* This);

    /*** IDispatch methods ***/
    HRESULT (STDMETHODCALLTYPE *GetTypeInfoCount)(
        IScriptControl* This,
        UINT *pctinfo);

    HRESULT (STDMETHODCALLTYPE *GetTypeInfo)(
        IScriptControl* This,
        UINT iTInfo,
        LCID lcid,
        ITypeInfo **ppTInfo);

    HRESULT (STDMETHODCALLTYPE *GetIDsOfNames)(
        IScriptControl* This,
        REFIID riid,
        LPOLESTR *rgszNames,
        UINT cNames,
        LCID lcid,
        DISPID *rgDispId);

    HRESULT (STDMETHODCALLTYPE *Invoke)(
        IScriptControl* This,
        DISPID dispIdMember,
        REFIID riid,
        LCID lcid,
        WORD wFlags,
        DISPPARAMS *pDispParams,
        VARIANT *pVarResult,
        EXCEPINFO *pExcepInfo,
        UINT *puArgErr);

    /*** IScriptControl methods ***/
    HRESULT (STDMETHODCALLTYPE *getLanguage)(
    	IScriptControl* This,
    	BSTR* pbstrLanguage);

    HRESULT (STDMETHODCALLTYPE *putLanguage)(
    	IScriptControl* This,
    	BSTR pbstrLanguage);

    HRESULT (STDMETHODCALLTYPE *getState)(
    	IScriptControl* This,
    	ScriptControlStates* pssState);

    HRESULT (STDMETHODCALLTYPE *putState)(
    	IScriptControl* This,
    	ScriptControlStates pssState);

    HRESULT (STDMETHODCALLTYPE *putSitehWnd)(
    	IScriptControl* This,
    	long phwnd);

    HRESULT (STDMETHODCALLTYPE *getSitehWnd)(
    	IScriptControl* This,
    	long* phwnd);

    HRESULT (STDMETHODCALLTYPE *getTimeout)(
    	IScriptControl* This,
    	long* plMilleseconds);

    HRESULT (STDMETHODCALLTYPE *putTimeout)(
    	IScriptControl* This,
    	long plMilleseconds);

    HRESULT (STDMETHODCALLTYPE *getAllowUI)(
    	IScriptControl* This,
    	VARIANT_BOOL* pfAllowUI);

    HRESULT (STDMETHODCALLTYPE *putAllowUI)(
    	IScriptControl* This,
    	VARIANT_BOOL pfAllowUI);

    HRESULT (STDMETHODCALLTYPE *getUseSafeSubset)(
    	IScriptControl* This,
    	VARIANT_BOOL* pfUseSafeSubset);

    HRESULT (STDMETHODCALLTYPE *putUseSafeSubset)(
    	IScriptControl* This,
    	VARIANT_BOOL pfUseSafeSubset);

    HRESULT (STDMETHODCALLTYPE *Modules)(
    	IScriptControl* This,
    	IScriptModuleCollection** ppmods);

    HRESULT (STDMETHODCALLTYPE *Error)(
    	IScriptControl* This,
    	IScriptError** ppse);

    HRESULT (STDMETHODCALLTYPE *CodeObject)(
    	IScriptControl* This,
    	IDispatch** ppdispObject);

    HRESULT (STDMETHODCALLTYPE *Procedures)(
    	IScriptControl* This,
    	IScriptProcedureCollection** ppdispProcedures);

    HRESULT (STDMETHODCALLTYPE *_AboutBox)(
    	IScriptControl* This);

    HRESULT (STDMETHODCALLTYPE *AddObject)(
    	IScriptControl* This,
        BSTR Name, 
        IDispatch* Object, 
        VARIANT_BOOL AddMembers);

    HRESULT (STDMETHODCALLTYPE *Reset)(
    	IScriptControl* This);

    HRESULT (STDMETHODCALLTYPE *AddCode)(
    	IScriptControl* This,
    	BSTR Code);

    HRESULT (STDMETHODCALLTYPE *Eval)(
    	IScriptControl* This,
        BSTR Expression, 
        VARIANT* pvarResult);

    HRESULT (STDMETHODCALLTYPE *ExecuteStatement)(
    	IScriptControl* This,
    	BSTR Statement);

    HRESULT (STDMETHODCALLTYPE *Run)(
    	IScriptControl* This,
        BSTR ProcedureName, 
        SAFEARRAY* Parameters, 
        VARIANT* pvarResult);

    END_INTERFACE
} IScriptControlVtbl;

interface IScriptControl {
    CONST_VTBL IScriptControlVtbl* lpVtbl;
};

/* incomplete. */

#endif