#include <amxmodx>
#include <cstrike>

#define PLUGIN "SHOP"
#define VERSION "1.1"
#define AUTHOR "Sugisaki"

#define NSTR(%1,%2) num_to_str(%1, %2, sizeof(%2) -1)

enum _:ITEMS
{
	ITEM_NAME[32],
	ITEM_COST,
	ITEM_TEAM,
	ITEM_FORWARD
}
enum _:TEAMS
{
	TEAM_TT = 1,
	TEAM_CT,
	TEAM_ALL
}

new const AVAILABLE[] = "y"
new const UNAVAILABLE[] = "r"

new Trie:g_Items, g_iTotalItems, g_iTotalTeamItems[TEAMS], g_MenuCallBack;

public plugin_init()
{
	register_plugin(PLUGIN, VERSION, AUTHOR)
	g_MenuCallBack = menu_makecallback("buymenu_callback")
	register_clcmd("say t", "open_shop")
}
public plugin_natives()
{
	g_Items = TrieCreate();
	register_native("shop_add_item", "_native_add_item")
}
public _native_add_item(pl, pr)
{
	//shop_add_item(const name[], cost, team = TEAM_ALL, const function[])
	if(!(1 <= get_param(3) <= TEAMS))
	{
		log_error(AMX_ERR_NATIVE, "Invalid Team")
		return;
	}
	new num[5], array[ITEMS], func[32];
	get_string(1, array[ITEM_NAME], charsmax(array[ITEM_NAME]));
	array[ITEM_COST] = get_param(2);
	array[ITEM_TEAM] = get_param(3);
	get_string(3, func, charsmax(func));
	if(get_func_id(func, pl) == -1)
	{
		log_error(AMX_ERR_NATIVE, "Invalid Function");
		return;
	}
	if((array[ITEM_FORWARD] = CreateOneForward(pl, func, FP_CELL)) == -1)
	{
		log_error(AMX_ERR_NATIVE, "Fail to create Function");
		return;
	}
	NSTR(g_iTotalItems, num);
	TrieSetArray(g_Items, num, array, ITEMS);

	g_iTotalTeamItems[get_param(3)] += 1;
	g_iTotalItems += 1;
}
public open_shop(id)
{
	new team = get_user_team(id);

	if(!g_iTotalTeamItems[team] && !g_iTotalTeamItems[TEAM_ALL])
	{
		client_print(id, print_chat, "No hay items para tu equipo");
		return;
	}
	static num[5], ItemArray[ITEMS], menu, i, item[50], money;
	money = cs_get_user_money(id);
	menu = menu_create("\rBuy menu", "buymenu_handle");
	for(i = 0 ; i < g_iTotalItems ; i++)
	{
		NSTR(i, num);
		TrieGetArray(g_Items, num, ItemArray, ITEMS);
		if(ItemArray[ITEM_TEAM] != team && ItemArray[ITEM_TEAM] != TEAM_ALL)
		{
			continue;
		}
		if(money >= ItemArray[ITEM_COST])
		{
			formatex(item, charsmax(item), "\w%s [\%s$%s\w]", ItemArray[ITEM_NAME], AVAILABLE, ItemArray[ITEM_COST])
		}
		else
		{
			formatex(item, charsmax(item), "%s \w[\%s$%s\w]", ItemArray[ITEM_NAME], UNAVAILABLE, ItemArray[ITEM_COST])
		}
		menu_additem(menu, item, num, 0, g_MenuCallBack)
	}
	menu_display(id, menu, 0, 60);
}
public buymenu_callback(id, menu, item)
{
	static a, c, num[5], ItemArray[ITEMS]
	menu_item_getinfo(menu, item, a, num, charsmax(num), "", 0, c);
	TrieGetArray(g_Items, num, ItemArray, ITEMS);
	if(ItemArray[ITEM_COST] > cs_get_user_money(id))
	{
		return ITEM_DISABLED
	}
	return ITEM_ENABLED
}
public buymenu_handle(id, menu, item)
{
	if(item == MENU_EXIT)
	{
		menu_destroy(menu);
		return;
	}
	static a, c, num[5], ItemArray[ITEMS], ret
	menu_item_getinfo(menu, item, a, num, charsmax(num), "", 0, c);
	menu_destroy(menu); 
	TrieGetArray(g_Items, num, ItemArray, ITEMS);
	if(ItemArray[ITEM_COST] > cs_get_user_money(id))
	{
		client_print(id, print_chat, "Dinero Insuficiente");
		return
	}
	ExecuteForward(ItemArray[ITEM_FORWARD], ret, id);
	if(ret == PLUGIN_HANDLED)
	{
		return;
	}
	cs_set_user_money(id, cs_get_user_money(id) - ItemArray[ITEM_COST]);
}

























