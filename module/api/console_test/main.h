// SPDX-FileCopyrightText: © 2024 Kim Eun-su <eunsu0402@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-linking-exception

#pragma once

#include "api.h"
#include "api_launchpad.h"
#include "api_project.h"
#include "api_unipack.h"
#include "api_workspace.h"

// secret.h은 파일 경로와 같은 빌드 환경에 따라 달라질 수 있는 정보를 담고 있습니다.
// 이 파일은 git에 올라가지 않으므로 테스트 코드에 따라 직접 수정하여 사용하십시오.
#include "secret.h"

int api_unipack_load_example();
int api_test2();
