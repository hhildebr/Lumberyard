/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

#ifndef CRYINCLUDE_EDITOR_PANELSIMPLETREEBROWSER_H
#define CRYINCLUDE_EDITOR_PANELSIMPLETREEBROWSER_H
#pragma once


#include "Controls/TreeCtrlReport.h"
#include "Viewport.h"

struct SSimpleTreeBrowserItem
{
    SSimpleTreeBrowserItem()
        : UserData(NULL)
        , Icon(-1)
    {
    }

    CString DisplayName;
    CString  Tooltip;
    void* UserData;
    int Icon;
    std::vector< SSimpleTreeBrowserItem > Children;
};
typedef std::vector< SSimpleTreeBrowserItem > TSimpleTreeBrowserItems;

struct ISimpleTreeBrowserScanner
{
    virtual ~ISimpleTreeBrowserScanner() {}

    virtual void Scan() = 0;
    virtual bool HasNewFiles() const = 0;
    virtual void GetScannedFiles(TSimpleTreeBrowserItems& items) const = 0;
    virtual bool IsScanningForFiles() const = 0;
};

class CSimpleTreeBrowser
    : public CXTResizeDialog
{
public:
    CSimpleTreeBrowser(int Height = 0, int iddImageList = 0, CWnd* pParent = NULL);   // standard constructor
    virtual ~CSimpleTreeBrowser();

    // Dialog Data
    enum
    {
        IDD = IDD_PANEL_SIMPLETREEBROWSER
    };

    void    Create(ISimpleTreeBrowserScanner* pScanner, CWnd* parent);

    void SetAutoResize(bool bEnabled, int minHeight = 0, int maxHeight  = 0);

    typedef Functor1< SSimpleTreeBrowserItem* > TEventCallback;
    void SetOnSelectCallback(TEventCallback& cb) { m_onSelectCallback = cb; };
    void SetOnDblClickCallback(TEventCallback& cb) { m_onDblClickCallback = cb; };
    void SetOnDragAndDropCallback(TEventCallback& cb, EViewportType type) { m_onDragAndDropCallbacks[type] = cb; };
    void EnableOnSelectCallback(bool bEnable) { m_bOnSelectCallbackEnabled = bEnable; };
    void EnableOnCblClickCallback(bool bEnable) { m_bOnDblClickCallbackEnabled = bEnable; };
    void EnableOnDragAndDropCallback(bool bEnable) { m_bOnDragAndDropCallbackEnabled = bEnable; };

    typedef Functor2wRet< SSimpleTreeBrowserItem*, CMenu&, bool > TContextMenuPopulationCallback;
    typedef Functor1< int > TContextMenuCommandCallback;
    void SetContextMenuCallbacks(TContextMenuPopulationCallback& cbCreate, TContextMenuCommandCallback& cbCommand) { m_CMCallbackCreate = cbCreate; m_CMCallbackCommand = cbCommand; }
    void EnableContextMenuCallbacks(bool bEnable) { m_bContextMenuCallbacksEnabled = bEnable; };

    void    Refresh();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnOK() {};
    virtual void OnCancel() {};

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnCustomBtnClick();
    afx_msg void OnFilterChange();
    DECLARE_MESSAGE_MAP()

    void OnSelectionChanged(CTreeItemRecord* pRec);
    void OnDragAndDrop(CTreeItemRecord* pRec);
    void OnDblclkBrowserTree(CTreeItemRecord* pRec);

private:
    void ClearTreeItems();
    void LoadFilesFromScanning();
    void AddItemsToTree(const TSimpleTreeBrowserItems& items, CTreeItemRecord* pParent = NULL);
    void ReselectLastSelectedTreeItem();
    void AutoResize();

private:
    CImageList m_cImageList;
    CTreeCtrlReport m_treeCtrl;
    CEdit m_filter;
    int m_iddImageList;
    int m_iHeight;
    CString m_lastSelectedRecord;
    bool m_bAutoResize;
    int m_iMinHeight;
    int m_iMaxHeight;

    ISimpleTreeBrowserScanner*  m_pScanner;
    TSimpleTreeBrowserItems m_CurrItems;
    CString m_CurrSelectedItem;

    TEventCallback m_onSelectCallback;
    TEventCallback m_onDblClickCallback;
    typedef std::map<EViewportType, TEventCallback> TDropEventCallbacks;
    TDropEventCallbacks m_onDragAndDropCallbacks;
    TContextMenuPopulationCallback m_CMCallbackCreate;
    TContextMenuCommandCallback m_CMCallbackCommand;

    bool m_bOnSelectCallbackEnabled;
    bool m_bOnDblClickCallbackEnabled;
    bool m_bOnDragAndDropCallbackEnabled;
    bool m_bContextMenuCallbacksEnabled;
};


#endif // CRYINCLUDE_EDITOR_PANELSIMPLETREEBROWSER_H
