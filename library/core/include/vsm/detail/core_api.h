#pragma once

#if vsm_config_shared_core
#	if vsm_detail_building_core
#		define vsm_detail_linkage_core vsm_dll_export
#	else
#		define vsm_detail_linkage_core vsm_dll_import
#	endif
#else
#	define vsm_detail_linkage_core
#endif
