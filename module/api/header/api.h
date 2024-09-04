// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "core.h"
#include "audio.h"
#include "launchpad.h"
#include "project.h"
#include "unipack.h"
#include "workspace.h"

namespace uniq::api
{
	template<typename T>
	class API_raii
	{
		std::shared_ptr<T> ptr_;
		API_raii() = default;
		explicit API_raii(std::shared_ptr<T> ptr) : ptr_(std::move(ptr)) {}
	public:
		explicit API_raii(const id_t id, const log::slm_t &location = log::slm_t{})
		{
			auto _ptr_o = core::ID_manager::get_shared_ptr_o<T>(id);
			if (!_ptr_o)
			{
				log::error("API call failed: invalid ID");
				return;
			}
			const auto& _ptr = _ptr_o.value();
			core::workspace_preset::workspace_info.set(_ptr->workspace_ID_get(), location);
			ptr_ = _ptr;
		}
		API_raii(API_raii&& other) noexcept = default;
		API_raii& operator=(API_raii&& other) noexcept = default;
		~API_raii() { if (ptr_) core::workspace_preset::workspace_info.reset(); }

		T* get() const { return ptr_.get(); }
		T& operator*() const { return *ptr_; }
		T* operator->() const { return ptr_.get(); }
		explicit operator bool() const { return static_cast<bool>(ptr_); }
		std::shared_ptr<T> get_shared_ptr() const { return ptr_; }
		static auto check_and_get(const id_t id, const log::slm_t &location = log::slm_t{})
		{
			return API_raii(id, location);
		}
	};
}