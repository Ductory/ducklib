#ifndef _DICTIONARY_H
#define _DICTIONARY_H

#include <oaidl.h>

#define C_DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

typedef interface IDictionary IDictionary;

C_DEFINE_GUID(CLSID_Dictionary, 0xEE09B103, 0x97E0, 0x11CF, 0x97, 0x8F, 0x00, 0xA0, 0x24, 0x63, 0xE0, 0x6F);
C_DEFINE_GUID(IID_IDictionary, 0x42C642C1, 0x97E1, 0x11CF, 0x97, 0x8F, 0x00, 0xA0, 0x24, 0x63, 0xE0, 0x6F);

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
	HRESULT (STDMETHODCALLTYPE *SetRefItem)(
		IDictionary* This,
		VARIANTARG *Key,
		VARIANTARG *pRetItem);

	HRESULT (STDMETHODCALLTYPE *SetItem)(
		IDictionary* This,
		VARIANTARG *Key,
		VARIANTARG *pRetItem);

	HRESULT (STDMETHODCALLTYPE *GetItem)(
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

	HRESULT (STDMETHODCALLTYPE *SetCompareMode)(
		IDictionary* This,
		CompareMethod pcomp);

	HRESULT (STDMETHODCALLTYPE *GetCompareMode)(
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

#endif