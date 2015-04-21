#pragma once

// Use to build a VARIANT which contains a SAFEARRAY of IDispatch*
// from an array of ActiveMIL objects.
// Usefull for methods such as IAVIFileHandler->Read().
//
// Usage: IDispatchArrayTYPE is the type of your array elements
//        such as CImage, IImagePtr(for native COM access classes users), 
//        or IImage(for MFC class wizard classes users).  
//        It must be an IDispatch derived type.
//
// e.g.: 
//    IImagePtr MyArray[10];
// 
//    // Allocate the images
//    //
//    MyArray[0] = m_Application1->CreateObject("MIL.Image", TRUE);
//    // ...
// 
//    MyAVIHandler->Read(CActiveMILObjectArray<IImagePtr>(MyArray, 10), 0, 10);
//

#include "OAIdl.h"

template <typename IDispatchArrayTYPE>class CActiveMILObjectsArray
{
public:
   CActiveMILObjectsArray(void** p, int nCount)
      {
      // fill the safearray with the IDispatch* passed as argument
      //
      SAFEARRAYBOUND rgsaboundsRead = { nCount, 0 };
      m_SafeArrayPtr = SafeArrayCreate(VT_DISPATCH, 1, &rgsaboundsRead);

      LPDISPATCH* ArrayDataPtr = NULL;
      SafeArrayAccessData(m_SafeArrayPtr, (VOID**)&ArrayDataPtr);

      for (int i= 0; i< nCount; i++)
         {
         ArrayDataPtr[i] = static_cast<IDispatch*>(p[i]);

         if (ArrayDataPtr[i])
            {
            ArrayDataPtr[i]->AddRef();
            }
         }

      SafeArrayUnaccessData(m_SafeArrayPtr);

      // wrap it with in a VARIANT
      //
      VariantInit(&m_Variant);
      V_VT(&m_Variant) = VT_ARRAY | VT_DISPATCH;
      V_ARRAY(&m_Variant) = m_SafeArrayPtr;
      }

   virtual ~CActiveMILObjectsArray()
      {
      // release our references to the abjects in the array
      //
      VariantClear(&m_Variant);
      }

   
   // cast operators to impersonate different ways to express an
   // OLE automation SAFEARRAY
   //
   operator VARIANT()
      {
      return m_Variant;
      }

   operator SAFEARRAY*()
      {
      return m_SafeArrayPtr;
      }

   // MS C++ native COM support
   //
   #ifdef _INC_COMUTIL
   operator _variant_t()
      {
      return m_Variant;
      }
   #endif

   // atl support
   //
   #ifdef __ATLBASE_H__
   operator CComVariant()
      {
      return m_Variant;
      }
   #endif

private:
   SAFEARRAY* m_SafeArrayPtr; 
   VARIANT m_Variant;
};

