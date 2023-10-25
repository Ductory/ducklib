#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include <oaidl.h>

#define C_DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

C_DEFINE_GUID(CLSID_Dictionary, 0xEE09B103, 0x97E0, 0x11CF, 0x97, 0x8F, 0x00, 0xA0, 0x24, 0x63, 0xE0, 0x6F);
C_DEFINE_GUID(IID_IDictionary, 0x42C642C1, 0x97E1, 0x11CF, 0x97, 0x8F, 0x00, 0xA0, 0x24, 0x63, 0xE0, 0x6F);

typedef interface IDictionary IDictionary;

typedef enum {
    BinaryCompare = 0,
    TextCompare = 1,
    DatabaseCompare = 2
} CompareMethod;

typedef struct IDictionaryVtbl {
	BEGIN_INTERFACE

	/*** IUnknown methods ***/
	HRESULT (STDMETHODCALLTYPE *QueryInterface)(
		IDictionary* This,
		REFIID riid,
		void **ppvObject);

	ULONG (STDMETHODCALLTYPE *AddRef)(
		IDictionary* This);

	ULONG (STDMETHODCALLTYPE *Release)(
		IDictionary* This);

	/*** IDispatch methods ***/
	HRESULT (STDMETHODCALLTYPE *GetTypeInfoCount)(
		IDictionary* This,
		UINT *pctinfo);

	HRESULT (STDMETHODCALLTYPE *GetTypeInfo)(
		IDictionary* This,
		UINT iTInfo,
		LCID lcid,
		ITypeInfo **ppTInfo);

	HRESULT (STDMETHODCALLTYPE *GetIDsOfNames)(
		IDictionary* This,
		REFIID riid,
		LPOLESTR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID *rgDispId);

	HRESULT (STDMETHODCALLTYPE *Invoke)(
		IDictionary* This,
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS *pDispParams,
		VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo,
		UINT *puArgErr);

	/*** IDictionary methods ***/
	HRESULT (STDMETHODCALLTYPE *putrefItem)(
		IDictionary* This,
		VARIANTARG *Key,
		VARIANTARG *pRetItem);

	HRESULT (STDMETHODCALLTYPE *putItem)(
		IDictionary* This,
		VARIANTARG *Key,
		VARIANTARG *pRetItem);

	HRESULT (STDMETHODCALLTYPE *getItem)(
		IDictionary* This,
		VARIANTARG *Key,
		VARIANTARG *pRetItem);

	HRESULT (STDMETHODCALLTYPE *Add)(
		IDictionary* This,
		VARIANTARG *Key,
		VARIANTARG *Item);

	HRESULT (STDMETHODCALLTYPE *Count)(
		IDictionary* This,
		LONG *pCount);

	HRESULT (STDMETHODCALLTYPE *Exists)(
		IDictionary* This,
		VARIANTARG *Key,
		VARIANT_BOOL *pExists);

	HRESULT (STDMETHODCALLTYPE *Items)(
		IDictionary* This,
		VARIANTARG *pItemsArray);

	HRESULT (STDMETHODCALLTYPE *Key)(
		IDictionary* This,
		VARIANTARG *Key,
		VARIANTARG *rhs);

	HRESULT (STDMETHODCALLTYPE *Keys)(
		IDictionary* This,
		VARIANTARG *pKeysArray);

	HRESULT (STDMETHODCALLTYPE *Remove)(
		IDictionary* This,
		VARIANTARG *Key);

	HRESULT (STDMETHODCALLTYPE *RemoveAll)(
		IDictionary* This);

	HRESULT (STDMETHODCALLTYPE *putCompareMode)(
		IDictionary* This,
		CompareMethod pcomp);

	HRESULT (STDMETHODCALLTYPE *getCompareMode)(
		IDictionary* This,
		CompareMethod *pcomp);

	HRESULT (STDMETHODCALLTYPE *_NewEnum)(
		IDictionary* This,
		IUnknown **ppunk);

	HRESULT (STDMETHODCALLTYPE *HashVal)(
		IDictionary* This,
		VARIANTARG *Key,
		VARIANTARG *HashVal);
} IDictionaryVtbl;
interface IDictionary {
	CONST_VTBL IDictionaryVtbl* lpVtbl;
};

#ifdef COBJMACROS
#ifndef WIDL_C_INLINE_WRAPPERS
/*** IUnknown methods ***/
#define IDoctionary_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IDoctionary_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IDoctionary_Release(This) (This)->lpVtbl->Release(This)
/*** IDispatch methods ***/
#define IDictionary_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define IDictionary_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define IDictionary_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define IDictionary_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
/*** IDictionary methods ***/
#define IDictionary_putrefItem(This,Key,pRetItem) (This)->lpVtbl->putrefItem(This,Key,pRetItem)
#define IDictionary_putItem(This,Key,pRetItem) (This)->lpVtbl->putItem(This,Key,pRetItem)
#define IDictionary_getItem(This,Key,pRetItem) (This)->lpVtbl->getItem(This,Key,pRetItem)
#define IDictionary_Add(This,Key,Item) (This)->lpVtbl->Add(This,Key,Item)
#define IDictionary_Count(This,pCount) (This)->lpVtbl->Count(This,pCount)
#define IDictionary_Exists(This,Key,pExists) (This)->lpVtbl->Exists(This,Key,pExists)
#define IDictionary_Items(This,pItemsArray) (This)->lpVtbl->Items(This,pItemsArray)
#define IDictionary_Key(This,Key,rhs) (This)->lpVtbl->Key(This,Key,rhs)
#define IDictionary_Keys(This,pKeysArray) (This)->lpVtbl->Keys(This,pKeysArray)
#define IDictionary_Remove(This,Key) (This)->lpVtbl->Remove(This,Key)
#define IDoctionary_RemoveAll(This) (This)->lpVtbl->RemoveAll(This)
#define IDictionary_putCompareMode(This,pcomp) (This)->lpVtbl->putCompareMode(This,pcomp)
#define IDictionary_getCompareMode(This,pcomp) (This)->lpVtbl->getCompareMode(This,pcomp)
#define IDictionary__NewEnum(This,ppunk) (This)->lpVtbl->_NewEnum(This,ppunk)
#define IDictionary_HashVal(This,Key,HashVal) (This)->lpVtbl->HashVal(This,Key,HashVal)
#else
/*** IUnknown methods ***/
static FORCEINLINE HRESULT IDictionary_QueryInterface(IDictionary* This,REFIID riid,void **ppvObject) {
    return This->lpVtbl->QueryInterface(This,riid,ppvObject);
}
static FORCEINLINE ULONG IDictionary_AddRef(IDictionary* This) {
    return This->lpVtbl->AddRef(This);
}
static FORCEINLINE ULONG IDictionary_Release(IDictionary* This) {
    return This->lpVtbl->Release(This);
}
/*** IDispatch methods ***/
static FORCEINLINE HRESULT IDictionary_GetTypeInfoCount(IDictionary* This,UINT *pctinfo) {
    return This->lpVtbl->GetTypeInfoCount(This,pctinfo);
}
static FORCEINLINE HRESULT IDictionary_GetTypeInfo(IDictionary* This,UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) {
    return This->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo);
}
static FORCEINLINE HRESULT IDictionary_GetIDsOfNames(IDictionary* This,REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) {
    return This->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId);
}
static FORCEINLINE HRESULT IDictionary_Invoke(IDictionary* This,DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) {
    return This->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr);
}
/* incomplete. */
#endif
#endif

#endif