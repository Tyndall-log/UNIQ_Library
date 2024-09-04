// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#include "api_unipack.h"

using namespace std;

namespace uniq::unipack
{
	using namespace api;

	API id_t unipack_load(const id_t workspace_id, const char* zip_path)
	{
		const API_raii<workspace::workspace> _workspace(workspace_id);
		if (!_workspace) return 0;
		log::info("Loading unipack from " + string(zip_path));
		const auto unipack = unipack::load(zip_path);
		if (!unipack)
		{
			log::error("unipack is nullptr");
			return 0;
		}
		_workspace->uniq_add(unipack);
		return unipack->ID_get();
	}
}