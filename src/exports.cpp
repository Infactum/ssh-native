/*
 *  Modern Native AddIn
 *  Copyright (C) 2018  Infactum
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <ComponentBase.h>
#include <types.h>

#include "SshAddIn.h"

#ifdef _WINDOWS
#pragma warning (disable : 4311 4302)
#endif

const WCHAR_T *GetClassNames() {
#if defined(_DEBUG) && defined(_WINDOWS)
    MessageBox(0, L"Attach Debugger", L"GetClassNames", MB_OK);
#endif
	static char16_t cls_names[] = u"SSH";
    return reinterpret_cast<WCHAR_T *>(cls_names);
}

long GetClassObject(const WCHAR_T *clsName, IComponentBase **pInterface) {
    if (!*pInterface) {
        auto cls_name = std::u16string(reinterpret_cast<const char16_t *>(clsName));
        if (cls_name == u"SSH") {
            *pInterface = new SshAddIn;
        }
        return (long) *pInterface;
    }
    return 0;
}

long DestroyObject(IComponentBase **pInterface) {
    if (!*pInterface) {
        return -1;
    }

    delete *pInterface;
    *pInterface = nullptr;
    return 0;
}

AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities) {
    return eAppCapabilitiesLast;
}
