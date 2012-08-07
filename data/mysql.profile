_checkchunk
_sanity
my_hash_sort_simple
code_state
_db_enter_
_db_return_
my_lengthsp_8bit
my_strnxfrm_simple
DoTrace
_db_pargs_
_db_keyword_
_db_doprnt_
_my_thread_var
ptr_compare_0
my_strnncollsp_simple
Field_string::pack(unsigned char*, unsigned char const*, unsigned int, bool)
make_char_array(unsigned char**, unsigned int, unsigned int, int)
Dbug_violation_helper::Dbug_violation_helper()
Dbug_violation_helper::leave()
safe_mutex_lock
ha_key_cmp
Item_func_between::val_int()
JOIN::optimize()
evaluate_join_record(JOIN*, st_join_table*, int)
Dbug_violation_helper::~Dbug_violation_helper()
hp_rec_hashnr
bmove512
mi_rnext
alloc_root
select_send::send_data(List<Item>&)
make_join_statistics(JOIN*, TABLE_LIST*, Item*, st_dynamic_array*)
safe_mutex_unlock
base_list_iterator::next_fast()
_mi_dpos
_mymalloc
Diagnostics_area::is_error() const
my_qsort2
bitmap_is_set
hp_write_key
_db_strict_keyword_
key_cmp(st_key_part_info*, unsigned char const*, unsigned int)
_mi_search_next
_mi_bin_search
QUICK_RANGE_SELECT::get_next()
_myfree
Field_long::val_int()
rr_quick(READ_RECORD*)
end_send(JOIN*, st_join_table*, bool)
base_list_iterator::init(base_list&)
JOIN::prepare(Item***, TABLE_LIST*, unsigned int, Item*, unsigned int, st_order*, st_order*, Item*, st_order*, st_select_lex*, st_select_lex_unit*)
String::free()
my_pread
net_write_buff(st_net*, unsigned char const*, unsigned long)
find_key_block
Field::is_null(long long)
open_table(THD*, TABLE_LIST*, st_mem_root*, bool*, unsigned int)
unlink_block
handler::read_multi_range_next(st_key_multi_range**)
mysql_execute_command(THD*)
my_real_read(st_net*, unsigned long*)
Item_param::val_int()
get_hash_link
JOIN::exec()
base_list_iterator::next()
link_block
my_strcasecmp_utf8
sub_select(JOIN*, st_join_table*, bool)
dispatch_command(enum_server_command, THD*, char*, unsigned int)
THD::is_error() const
update_ref_and_keys(THD*, st_dynamic_array*, st_join_table*, unsigned int, Item*, COND_EQUAL*, unsigned long long, st_select_lex*, st_sargable_param**)
my_net_write
set_thd_proc_info
Item_field::val_int()
do_add
handler::ha_statistic_increment(unsigned long system_status_var::*) const
_current_thd()
Protocol::net_store_data(unsigned char const*, unsigned long)
free_root
open_tables(THD*, TABLE_LIST**, unsigned int*, unsigned int)
key_cache_read
reinit_stmt_before_use(THD*, st_lex*)
Protocol::send_fields(List<Item>*, unsigned int)
Field_string::val_str(String*, String*)
my_wc_mb_latin1
find_all_keys(st_sort_param*, SQL_SELECT*, unsigned char**, st_io_cache*, st_io_cache*, st_io_cache*)
Field_long::cmp(unsigned char const*, unsigned char const*)
_mi_search
Item_func::fix_fields(THD*, Item**)
insert_params_with_log(Prepared_statement*, unsigned char*, unsigned char*, unsigned char*, String*)
unreg_request
_mi_pack_key
Item_field::fix_fields(THD*, Item**)
Prepared_statement::execute(String*, bool)
String::ptr() const
String::length() const
copy_and_convert(char*, unsigned int, charset_info_st*, char const*, unsigned int, charset_info_st*, unsigned int*)
MYSQLparse(void*)
Item::Item()
Item_field::used_tables() const
Field::send_binary(Protocol*)
_mi_read_static_record
_mi_kpos
do_command(THD*)
_db_dump_
_mi_check_index
handler::read_range_next()
base_list_iterator::base_list_iterator(base_list&)
setup_conds(THD*, TABLE_LIST*, TABLE_LIST*, Item**)
make_join_select(JOIN*, SQL_SELECT*, Item*)
base_list::empty()
make_sortkey(st_sort_param*, unsigned char*, unsigned char*)
sql_alloc(unsigned long)
add_key_fields(JOIN*, key_field_t**, unsigned int*, Item*, unsigned long long, st_sargable_param**)
Protocol_binary::store(Field*)
_mi_get_static_key
PROFILING::status_change(char const*, char const*, char const*, unsigned int)
create_ref_for_key(JOIN*, st_join_table*, keyuse_t*, unsigned long long)
add_key_field(key_field_t**, unsigned int, Item_func*, Field*, bool, Item**, unsigned int, unsigned long long, st_sargable_param**)
Item_sum_sum::add()
Protocol::store_string_aux(char const*, unsigned long, charset_info_st*, charset_info_st*)
Item_func::type() const
do_select(JOIN*, List<Item>*, st_table*, Procedure*)
Prepared_statement::execute_loop(String*, bool, unsigned char*, unsigned char*)
execute_sqlcom_select(THD*, TABLE_LIST*)
get_best_combination(JOIN*)
heap_write
String::length(unsigned int)
find_field_in_tables(THD*, Item_ident*, TABLE_LIST*, TABLE_LIST*, Item**, find_item_error_report_type, bool, bool)
ha_release_temporary_latches(THD*)
ha_myisam::index_next(unsigned char*)
setup_fields(THD*, Item**, List<Item>&, enum_mark_columns, List<Item>*, bool)
String::String(char*, unsigned int, charset_info_st*)
Item_param::used_tables() const
vio_read
mysql_select(THD*, Item***, TABLE_LIST*, unsigned int, List<Item>&, Item*, unsigned int, st_order*, st_order*, Item*, st_order*, unsigned long long, select_result*, st_select_lex_unit*, st_select_lex*)
close_thread_tables(THD*)
Item_field::set_field(Field*)
cleanup_items(Item*)
SQL_SELECT::test_quick_select(THD*, Bitmap<64u>, unsigned long long, unsigned long, bool)
unpack_addon_fields(st_sort_addon_field*, unsigned char*)
check_grant(THD*, unsigned long, TABLE_LIST*, unsigned int, unsigned int, bool)
mi_nommap_pread
hp_find_block
my_utf8_uni
hp_rec_key_cmp
my_store_ptr
my_charset_same
mi_rkey
net_store_length
mysqld_stmt_execute(THD*, char*, unsigned int)
_mi_fetch_keypage
String::copy(char const*, unsigned int, charset_info_st*, charset_info_st*, unsigned int*)
Item_bool_func2::fix_length_and_dec()
String::set(char*, unsigned int, charset_info_st*)
check_stack_overrun(THD*, long, unsigned char*)
my_longlong10_to_str_8bit
check_table_access(THD*, unsigned long, TABLE_LIST*, unsigned int, bool)
Arg_comparator::set_cmp_func(Item_result_field*, Item**, Item**, Item_result)
_mi_test_if_changed
check_ptr
lock_tables(THD*, TABLE_LIST*, unsigned int, bool*)
handler::compare_key(st_key_range*)
handle_select(THD*, st_lex*, select_result*, unsigned long)
DTCollation::set_repertoire_from_charset(charset_info_st*)
find_field_in_table(THD*, st_table*, char const*, unsigned int, bool, unsigned int*)
copy_fields(TMP_TABLE_PARAM*)
hp_mask
setup_tables(THD*, Name_resolution_context*, List<TABLE_LIST>*, TABLE_LIST*, TABLE_LIST**, bool)
Protocol::write()
st_select_lex::master_unit()
Field::pack(unsigned char*, unsigned char const*)
Field::key_cmp(unsigned char const*, unsigned int)
Item::const_item() const
st_table::column_bitmaps_set(st_bitmap*, st_bitmap*)
ull2dec
Protocol_binary::store(char const*, unsigned long, charset_info_st*)
Arg_comparator::set_compare_func(Item_result_field*, Item_result)
add_key_part(st_dynamic_array*, key_field_t*)
make_cond_for_table(Item*, unsigned long long, unsigned long long)
String::~String()
handler::ha_reset()
hp_delete_key
remove_reader
alloc_query(THD*, char const*, unsigned int)
init_alloc_root
Field_string::unpack(unsigned char*, unsigned char const*, unsigned int, bool)
Item::cleanup()
Field_string::sort_string(unsigned char*, unsigned int)
join_read_const_table(st_join_table*, st_position*)
Field::maybe_null()
Item_field::result_type() const
JOIN::cleanup(bool)
mi_status
next_free_record_pos
end_write(JOIN*, st_join_table*, bool)
String::real_alloc(unsigned int)
Field::handle_int32(unsigned char*, unsigned char const*, bool, bool)
Item_field::send(Protocol*, String*)
Field::make_field(Send_field*)
check_simple_equality(Item*, Item*, Item*, COND_EQUAL*)
thr_end_alarm
Prepared_statement::set_parameters(String*, unsigned char*, unsigned char*)
ha_myisam::info(unsigned int)
String::String()
Protocol_binary::prepare_for_resend()
get_mm_leaf(RANGE_OPT_PARAM*, Item*, Field*, st_key_part*, Item_func::Functype, Item*)
net_real_write
List_iterator_fast<Item>::operator++(int)
update_depend_map(JOIN*)
Field::unpack(unsigned char*, unsigned char const*)
st_select_lex_unit::first_select()
build_equal_items(THD*, Item*, COND_EQUAL*, List<TABLE_LIST>*, COND_EQUAL**)
setup_without_group(THD*, Item**, TABLE_LIST*, TABLE_LIST*, List<Item>&, List<Item>&, Item**, st_order*, st_order*, bool*)
build_equal_items_for_cond(THD*, Item*, COND_EQUAL*)
st_table::set_keyread(bool)
make_select(st_table*, unsigned long long, unsigned long long, Item*, bool, int*)
init_dynamic_array2
update_sum_func(Item_sum**)
vio_blocking
Item_field::make_field(Send_field*)
select_send::send_fields(List<Item>&, unsigned int)
String::alloc(unsigned int)
String::set(char const*, unsigned int, charset_info_st*)
List_iterator_fast<Item>::List_iterator_fast(List<Item>&)
cmp_db_names(char const*, char const*)
Discrete_intervals_list::empty()
vio_is_blocking
make_join_readinfo(JOIN*, unsigned long long)
Item::real_item()
Bitmap<64u>::clear_all()
st_select_lex::cleanup()
create_tmp_table(THD*, TMP_TABLE_PARAM*, List<Item>&, st_order*, bool, bool, unsigned long long, unsigned long, char*)
MYSQLlex(void*, void*)
close_cached_file
base_list::base_list()
Item_func::update_used_tables()
String::realloc(unsigned int)
JOIN::join_free()
remove_const(JOIN*, st_order*, Item*, bool, bool*)
Query_tables_list::uses_stored_routines() const
Item_equal::functype() const
Query_cache::send_result_to_client(THD*, char*, unsigned int)
join_read_const(st_join_table*)
Item_param::query_val_str(THD*, String*) const
open_and_lock_tables_derived(THD*, TABLE_LIST*, bool)
Query_arena::free_items()
Field_str::charset() const
Item_param::type() const
Protocol_text::store(char const*, unsigned long, charset_info_st*, charset_info_st*)
JOIN::init(THD*, List<Item>&, unsigned long long, select_result*)
check_quick_keys(PARAM*, unsigned int, SEL_ARG*, unsigned char*, unsigned int, int, unsigned char*, unsigned int, int)
Field::val_str(String*)
String::needs_conversion(unsigned int, charset_info_st*, charset_info_st*, unsigned int*)
mysql_reset_thd_for_next_command(THD*)
Field::set_notnull(long long)
Statement_map::find(unsigned long)
Field_long::pack(unsigned char*, unsigned char const*, unsigned int, bool)
net_end_statement(THD*)
Field_long::store(long long, bool)
TABLE_LIST::reinit_before_use(THD*)
my_qsort
Statement::set_statement(Statement*)
handler::ha_write_row(unsigned char*)
mark_used_tables_as_free_for_reuse(THD*, st_table*)
THD::cleanup_after_query()
substitute_for_best_equal_field(Item*, COND_EQUAL*, void*)
String::set_int(long long, bool, charset_info_st*)
PROFILING::start_new_query(char const*)
Query_arena::alloc(unsigned long)
handler::column_bitmaps_signal()
check_column_grant_in_table_ref(THD*, TABLE_LIST*, char const*, unsigned int)
Item_field::type() const
mi_reset
Field::new_field(st_mem_root*, st_table*, bool)
test_if_group_changed(List<Cached_item>&)
init_read_record(READ_RECORD*, THD*, st_table*, SQL_SELECT*, int, bool, bool)
find_prepared_statement(THD*, unsigned long)
mysql_handle_derived(st_lex*, bool (*)(THD*, st_lex*, TABLE_LIST*))
count_field_types(st_select_lex*, TMP_TABLE_PARAM*, List<Item>&, bool)
Item_int::val_int()
update_const_equal_items(Item*, st_join_table*)
setup_procedure(THD*, st_order*, select_result*, List<Item>&, int*)
optimize_cond(JOIN*, Item*, List<TABLE_LIST>*, Item::cond_result*)
Prepared_statement::cleanup_stmt()
item_cmp_type(Item_result, Item_result)
Item_field::cleanup()
inc_counter_for_resize_op
bitmap_is_set_all
thr_alarm
ha_heap::write_row(unsigned char*)
Arg_comparator::can_compare_as_dates(Item*, Item*, unsigned long long*)
Arg_comparator::compare_int_unsigned_signed()
my_net_read
setup_tables_and_check_access(THD*, Name_resolution_context*, List<TABLE_LIST>*, TABLE_LIST*, TABLE_LIST**, bool, unsigned long, unsigned long)
MYSQL_LOG::is_open()
setup_natural_join_row_types(THD*, List<TABLE_LIST>*, Name_resolution_context*)
Item::delete_self()
get_mm_tree(RANGE_OPT_PARAM*, Item*)
Item_bool_func2::cleanup()
Bitmap<64u>::Bitmap()
Item_param::save_in_field(Field*, bool)
check_one_key
end_send_group(JOIN*, st_join_table*, bool)
String::shrink(unsigned int)
Item_field::field_type() const
handle_one_connection
Field_long::unpack(unsigned char*, unsigned char const*, unsigned int, bool)
create_table_def_key(THD*, char*, TABLE_LIST*, bool)
set_param_int32(Item_param*, unsigned char**, unsigned long)
Item::is_datetime()
Field_long::result_type() const
best_extension_by_limited_search(JOIN*, unsigned long long, unsigned int, double, double, unsigned int, unsigned int)
thd_increment_bytes_received
Field::eq(Field*)
remove_eq_conds(THD*, Item*, Item::cond_result*)
st_join_table::cleanup()
setup_table_map(st_table*, TABLE_LIST*, unsigned int)
st_select_lex_unit::cleanup()
hp_find_free_hash
Item_basic_constant::cleanup()
Item_param::result_type() const
List<Item>::List()
delete_dynamic
Item_equal::update_used_tables()
DTCollation::DTCollation(charset_info_st*, Derivation)
Arg_comparator::is_owner_equal_func()
bitmap_fast_test_and_set
st_select_lex_unit::is_union()
String::copy(char const*, unsigned int, charset_info_st*)
DTCollation::set(charset_info_st*, Derivation)
_mi_search_pos
update_field_dependencies(THD*, Field*, st_table*)
String::replace(unsigned int, unsigned int, char const*, unsigned int)
get_mm_parts(RANGE_OPT_PARAM*, Item*, Field*, Item_func::Functype, Item*, Item_result)
THD_TRANS::reset()
Diagnostics_area::reset_diagnostics_area()
Bitmap<64u>::intersect(Bitmap<64u>&)
Field_num::val_decimal(my_decimal*)
log_slow_statement(THD*)
Field_string::type() const
best_access_path(JOIN*, st_join_table*, THD*, unsigned long long, unsigned int, double, double)
_myrealloc
THD::set_statement(Statement*)
Sql_alloc::operator new(unsigned long)
filesort(THD*, st_table*, st_sort_field*, unsigned int, SQL_SELECT*, unsigned long, bool, unsigned long*)
queue_insert
Query_cache::store_query(THD*, TABLE_LIST*)
ha_autocommit_or_rollback(THD*, int)
store_key::copy()
Field::pack_int32(unsigned char*, unsigned char const*, bool)
PROFILING::finish_current_query()
get_key_scans_params(PARAM*, SEL_TREE*, bool, bool, double)
rr_unpack_from_buffer(READ_RECORD*)
store_key_item::copy_inner()
TABLE_LIST::placeholder()
Arg_comparator::set_cmp_func(Item_result_field*, Item**, Item**, bool)
setup_end_select_func(JOIN*)
THD::killed_errno() const
List_iterator<TABLE_LIST>::List_iterator(List<TABLE_LIST>&)
make_outerjoin_info(JOIN*)
TABLE_LIST::process_index_hints(st_table*)
TMP_TABLE_PARAM::cleanup()
reset_nj_counters(List<TABLE_LIST>*)
handler::mark_trx_read_write()
Statement::set_n_backup_statement(Statement*, Statement*)
get_quick_keys(PARAM*, QUICK_RANGE_SELECT*, st_key_part*, SEL_ARG*, unsigned char*, unsigned int, unsigned char*, unsigned int)
Item_field::val_decimal(my_decimal*)
setup_order(THD*, Item**, TABLE_LIST*, List<Item>&, List<Item>&, st_order*)
Item_result_field::cleanup()
query_cache_end_of_result(THD*)
Item_equal::get_const()
st_select_lex::first_inner_unit()
JOIN::alloc_func_list()
handler::ha_thd() const
copy_funcs(Item**, THD const*)
Statement::query_length()
Sql_alloc::Sql_alloc()
key_and(RANGE_OPT_PARAM*, SEL_ARG*, SEL_ARG*, unsigned int)
reset_stmt_params(Prepared_statement*)
base_ilist::is_empty()
Query_tables_list::first_not_own_table()
mysql_reset_errors(THD*, bool)
THD::charset()
Item_func_between::fix_length_and_dec()
release_whole_queue
my_hash_sort_bin
sp_cache_flush_obsolete(sp_cache**)
mysql_opt_change_db(THD*, st_mysql_lex_string const*, st_mysql_lex_string*, bool, bool*)
String::alloced_length() const
Item_func::used_tables() const
base_list::is_empty()
do_field_eq(Copy_field*)
THD::reset_current_stmt_binlog_row_based()
Bitmap<64u>::merge(Bitmap<64u>&)
st_lex::need_correct_ident()
_downheap
Bitmap<64u>::is_set(unsigned int) const
check_lock_and_start_stmt(THD*, st_table*, thr_lock_type)
get_best_group_min_max(PARAM*, SEL_TREE*)
Item::check_cols(unsigned int)
_mi_keynr
thd_increment_bytes_sent
Query_tables_list::requires_prelocking()
eliminate_item_equal(Item*, COND_EQUAL*, Item_equal*)
QUICK_RANGE_SELECT::QUICK_RANGE_SELECT(THD*, st_table*, unsigned int, bool, st_mem_root*)
Query_arena::calloc(unsigned long)
List<Cached_item>::delete_elements()
Field::unpack_int32(unsigned char*, unsigned char const*, bool)
net_send_eof(THD*, unsigned int, unsigned int)
reg_requests
SQL_SELECT::cleanup()
store_key::store_key(THD*, Field*, unsigned char*, unsigned char*, unsigned int)
propagate_cond_constants(THD*, I_List<COND_CMP>*, Item*, Item*)
query_cache_insert(st_net*, char const*, unsigned long)
Arg_comparator::cache_converted_constant(THD*, Item**, Item**, Item_result)
String::replace(unsigned int, unsigned int, String const&)
calc_group_buffer(JOIN*, st_order*)
Item::val_uint()
Sql_alloc::operator delete(void*, unsigned long)
end_read_record(READ_RECORD*)
check_equality(THD*, Item*, COND_EQUAL*, List<Item>*)
bmove_upp
Item::fix_fields(THD*, Item**)
deny_updates_if_read_only_option(THD*, TABLE_LIST*)
Arg_comparator::try_year_cmp_func(Item_result)
base_list::~base_list()
Item::operator new(unsigned long)
write_eof_packet(THD*, st_net*, unsigned int, unsigned int)
vio_write
net_flush
get_hash_symbol(char const*, unsigned int, bool)
queue_remove
_mi_record_pos
cmp_item::get_comparator(Item_result, charset_info_st*)
Discrete_intervals_list::set_members(Discrete_interval*, Discrete_interval*, Discrete_interval*, unsigned int)
List<Cached_item>::List()
Field_long::type() const
Item_func::arguments() const
Item_result_field::Item_result_field()
LOGGER::log_command(THD*, enum_server_command)
THD::clear_error()
check_and_update_table_version(THD*, TABLE_LIST*, st_table_share*)
SQL_SELECT::~SQL_SELECT()
THD::is_valid_time()
setup_ftfuncs(st_select_lex*)
ha_myisam::reset()
heap_scan
Protocol::init(THD*)
Item_param::set_int(long long, unsigned int)
Item_func::transform(Item* (Item::*)(unsigned char*), unsigned char*)
dec_counter_for_resize_op
sel_cmp(Field*, unsigned char*, unsigned char*, unsigned char, unsigned char)
check_result(unsigned int, int)
THD::vio_ok() const
init_ftfuncs(THD*, st_select_lex*, bool)
mark_temp_tables_as_free_for_reuse(THD*)
st_lex::is_ps_or_view_context_analysis()
general_log_write(THD*, enum_server_command, char const*, unsigned int)
Item_func_eq::functype() const
QUICK_RANGE_SELECT::reset()
ha_myisam::position(unsigned char const*)
mi_records_in_range
SQL_SELECT::skip_record(THD*, bool*)
handler::ha_index_or_rnd_end()
Field::new_key_field(st_mem_root*, st_table*, unsigned char*, unsigned char*, unsigned int)
Item_func_between::functype() const
Ha_trx_info::is_started() const
Item_equal_iterator::Item_equal_iterator(Item_equal&)
Item_equal::update_const()
Diagnostics_area::set_eof_status(THD*)
JOIN::destroy()
show_system_thread(enum_thread_type)
Item::~Item()
Arg_comparator::cleanup()
Item_param::reset()
st_select_lex::fix_prepare_information(THD*, Item**, Item**)
DTCollation::DTCollation()
Item_num::Item_num()
st_select_lex_unit::set_limit(st_select_lex*)
st_table::clear_column_bitmaps()
Item_func::compile(bool (Item::*)(unsigned char**), unsigned char**, Item* (Item::*)(unsigned char*), unsigned char*)
Bitmap<64u>::is_clear_all() const
get_sort_by_table(st_order*, st_order*, TABLE_LIST*)
st_table::default_column_bitmaps()
my_micro_time_and_time
Item_func_eq::val_int()
TMP_TABLE_PARAM::init()
THD::set_time()
Item::not_null_tables() const
TMP_TABLE_PARAM::TMP_TABLE_PARAM()
join_init_read_record(st_join_table*)
rr_from_pointers(READ_RECORD*)
fill_used_fields_bitmap(PARAM*)
select_result::prepare(List<Item>&, st_select_lex_unit*)
make_leaves_list(TABLE_LIST**, TABLE_LIST*)
Item_ident::cleanup()
SQL_SELECT::SQL_SELECT()
base_list::push_back(void*)
handler::start_stmt(THD*, thr_lock_type)
heap_check_heap
I_List<Item_change_record>::is_empty()
PROFILING::set_query_source(char*, unsigned int)
save_index(st_sort_param*, unsigned char**, unsigned int, st_filesort_info*)
Item_func::const_item() const
create_sort_index(THD*, JOIN*, st_order*, unsigned long, unsigned long, bool)
init_sql_alloc(st_mem_root*, unsigned int, unsigned int)
setup_group(THD*, Item**, TABLE_LIST*, List<Item>&, List<Item>&, st_order*, bool*)
tree_and(RANGE_OPT_PARAM*, SEL_TREE*, SEL_TREE*)
my_net_set_read_timeout
Item_int::~Item_int()
heap_rrnd
Diagnostics_area::status() const
ilink::ilink()
free_io_cache(st_table*)
decimal_add
store_key_item::store_key_item(THD*, Field*, unsigned char*, unsigned char*, unsigned int, Item*)
COND_EQUAL::COND_EQUAL()
THD::set_query(char*, unsigned int)
Item_equal::compare_const(Item*)
JOIN::JOIN(THD*, List<Item>&, unsigned long long, select_result*)
get_quick_select(PARAM*, unsigned int, SEL_ARG*, st_mem_root*)
setup_conversion_functions(Prepared_statement*, unsigned char**, unsigned char*)
mi_position
I_List_iterator<Item_change_record>::I_List_iterator(I_List<Item_change_record>&)
Query_arena::is_stmt_prepare() const
Item::copy_andor_structure(THD*)
my_micro_time
check_quick_select(PARAM*, unsigned int, SEL_ARG*, bool)
JOIN::is_top_level_join() const
Item_param::eq(Item const*, bool) const
my_decimal::init()
get_full_func_mm_tree(RANGE_OPT_PARAM*, Item_func*, Item_field*, Item*, bool)
Sql_alloc::~Sql_alloc()
Statement::set_query_inner(char*, unsigned int)
select_send::send_eof()
choose_plan(JOIN*, unsigned long long)
Item::update_used_tables()
add_not_null_conds(JOIN*)
filesort_free_buffers(st_table*, bool)
Field_str::decimals() const
base_ilist_iterator::base_ilist_iterator(base_ilist&)
TABLE_LIST::first_leaf_for_name_resolution()
handler::read_multi_range_first(st_key_multi_range**, st_key_multi_range*, unsigned int, bool, st_handler_buffer*)
base_list::remove(list_node**)
find_item_equal(COND_EQUAL*, Field*, bool*)
test_if_subpart(st_order*, st_order*)
lex_start(THD*)
st_join_table::is_using_loose_index_scan()
TABLE_LIST::is_table_ref_id_equal(st_table_share*) const
mi_extra
handler::read_range_first(st_key_range const*, st_key_range const*, bool, bool)
Bitmap<64u>::set_bit(unsigned int)
Item::eq_cmp_result() const
base_list::head()
Item_equal::Item_equal(Item*, Item_field*)
TABLE_LIST::is_leaf_for_name_resolution()
select_send::cleanup()
calc_hash
compare_ulong
my_decimal_add(unsigned int, my_decimal*, my_decimal const*, my_decimal const*)
Protocol_text::Protocol_text(THD*)
Field::can_be_compared_as_longlong() const
QUICK_RANGE_SELECT::~QUICK_RANGE_SELECT()
set_position(JOIN*, unsigned int, st_join_table*, keyuse_t*)
binlog_log_row(st_table*, unsigned char const*, unsigned char const*, bool (*)(THD*, st_table*, bool, st_bitmap*, unsigned int, unsigned char const*, unsigned char const*))
handler::ha_table_flags() const
Prepared_statement::type() const
mysql_parse(THD*, char*, unsigned int, char const**)
Arg_comparator::~Arg_comparator()
bitmap_init
st_select_lex::setup_ref_array(THD*, unsigned int)
ha_myisam::index_read_idx_map(unsigned char*, unsigned int, unsigned char const*, unsigned long, ha_rkey_function)
hashcmp
st_select_lex_unit::reinit_exec_mechanism()
THD::rollback_item_tree_changes()
select_result::initialize_tables(JOIN*)
THD::fill_derived_tables()
Item_func_eq::~Item_func_eq()
Name_resolution_context::resolve_in_table_list_only(TABLE_LIST*)
ulonglong2decimal
st_table_share::get_table_ref_type() const
is_param_null(unsigned char const*, unsigned long)
tmp_use_all_columns(st_table*, st_bitmap*)
insert_dynamic
open_and_lock_tables(THD*, TABLE_LIST*)
List_iterator<Item>::List_iterator(List<Item>&)
Bitmap<64u>::init()
List_iterator<Item_field>::List_iterator(List<Item_field>&)
init_io_cache
Query_arena::memdup_w_gap(void const*, unsigned long, unsigned int)
List<Item_equal>::head()
vio_should_retry
my_strnncoll_binary
Protocol_binary::prepare_for_send(List<Item>*)
Bitmap<64u>::init(unsigned int)
Item::operator delete(void*, unsigned long)
Item::save_in_field_no_warnings(Field*, bool)
sortlength(THD*, st_sort_field*, unsigned int, bool*)
int2my_decimal(unsigned int, long long, char, my_decimal*)
agg_cmp_type(Item_result*, Item**, unsigned int)
Item_param::field_type() const
Item_equal::fix_length_and_dec()
Bitmap<64u>::set_prefix(unsigned int)
Item_bool_rowready_func2::~Item_bool_rowready_func2()
Statement::query()
multi_alloc_root
memdup_root
Diagnostics_area::total_warn_count() const
Item_equal::sort(int (*)(Item_field*, Item_field*, void*), void*)
List<Item>::~List()
Item_result_field::~Item_result_field()
Item_func::Item_func(Item*, Item*)
my_time
Item_param::convert_str_value(THD*)
Item_int::Item_int(long long, unsigned int)
ilink::~ilink()
Diagnostics_area::server_status() const
base_list::pop()
Item_bool_func2::Item_bool_func2(Item*, Item*)
my_hash_first
ha_heap::rnd_pos(unsigned char*, unsigned char*)
Query_arena::Query_arena()
st_lex::which_check_option_applicable()
ha_myisam::index_flags(unsigned int, unsigned int, bool) const
st_lex::only_view_structure()
JOIN::make_sum_func_list(List<Item>&, List<Item>&, bool, bool)
SEL_ARG::store_min(unsigned int, unsigned char**, unsigned int)
Field_str::derivation() const
Lex_input_stream::yyGet()
Diagnostics_area::is_set() const
Field_num::decimals() const
find_item_in_list(Item*, List<Item>&, unsigned int*, find_item_error_report_type, enum_resolution_type*)
Protocol::~Protocol()
List<Item_equal>::push_back(Item_equal*)
SEL_ARG::SEL_ARG(Field*, unsigned char const*, unsigned char const*)
Item::transform(Item* (Item::*)(unsigned char*), unsigned char*)
check_table_binlog_row_based(THD*, st_table*)
Item_equal::~Item_equal()
base_ilist_iterator::next()
Arg_comparator::Arg_comparator()
Item_num::~Item_num()
handler::scan_time()
Item::compile(bool (Item::*)(unsigned char**), unsigned char**, Item* (Item::*)(unsigned char*), unsigned char*)
Item_basic_constant::Item_basic_constant()
Statement::~Statement()
get_quick_record_count(THD*, SQL_SELECT*, st_table*, Bitmap<64u> const*, unsigned long)
set_dynamic
Field::optimize_range(unsigned int, unsigned int)
TABLE_LIST::prepare_where(THD*, Item**, bool)
check_result_and_overflow(unsigned int, int, my_decimal*)
ha_heap::create(char const*, st_table*, st_ha_create_information*)
get_lock_data(THD*, st_table**, unsigned int, unsigned int, st_table**)
my_decimal::sanity_check()
List_iterator<TABLE_LIST>::operator++(int)
st_table::mark_columns_used_by_index_no_reset(unsigned int, st_bitmap*)
select_result::prepare2()
create_last_word_mask
st_lex::is_single_level_stmt()
List_iterator<Item>::operator++(int)
dbug_tmp_use_all_columns(st_table*, st_bitmap*)
Item_int_func::~Item_int_func()
get_addon_fields(THD*, Field**, unsigned int, unsigned int*)
Protocol::Protocol(THD*)
update_depend_map(JOIN*, st_order*)
Item_basic_constant::~Item_basic_constant()
st_lex::first_lists_tables_same()
Item_equal::get_first()
get_func_mm_tree(RANGE_OPT_PARAM*, Item_func*, Field*, Item*, Item_result, bool)
setup_wild(THD*, TABLE_LIST*, List<Item>&, List<Item>*, unsigned int)
mi_lock_database
vio_errno
Item_bool_func2::set_cmp_func()
cmp_item_int::cmp_item_int()
cp_buffer_from_ref(THD*, st_table*, st_table_ref*)
Reprepare_observer::reset_reprepare_observer()
Item_func::~Item_func()
Item_func_between::fix_fields(THD*, Item**)
test_if_skip_sort_order(st_join_table*, st_order*, unsigned long, bool, Bitmap<64u>*)
heap_create
Item_equal::add(Item*)
find_order_in_list(THD*, Item**, TABLE_LIST*, st_order*, List<Item>&, List<Item>&, bool)
Item_sum_num::fix_fields(THD*, Item**)
stored_field_cmp_to_item(THD*, Field*, Item*)
ilink::unlink()
my_count_bits
COND_EQUAL::~COND_EQUAL()
base_ilist::empty()
bitmap_copy
String::charset() const
hp_movelink
List_iterator_fast<Item_field>::List_iterator_fast(List<Item_field>&)
Item_func_eq::Item_func_eq(Item*, Item*)
Statement::Statement()
my_decimal::my_decimal()
List_iterator<Cached_item>::List_iterator(List<Cached_item>&)
Field::derivation() const
opt_sum_query(THD*, TABLE_LIST*, List<Item>&, Item*)
st_select_lex::next_select_in_list()
Arg_comparator::Arg_comparator(Item**, Item**)
st_select_lex::add_table_to_list(THD*, Table_ident*, st_mysql_lex_string*, unsigned long, thr_lock_type, List<Index_hint>*, st_mysql_lex_string*)
is_local_field(Item*)
ha_myisam::index_read_map(unsigned char*, unsigned char const*, unsigned long, ha_rkey_function)
register_used_fields(st_sort_param*)
Bitmap<64u>::is_subset(Bitmap<64u> const&) const
Item_field::replace_equal_field(unsigned char*)
JOIN::cleanup_item_list(List<Item>&) const
free_tmp_table(THD*, st_table*)
QUICK_RANGE::make_min_endpoint(st_key_range*)
make_unireg_sortorder(st_order*, unsigned int*, st_sort_field*)
Prepared_statement::close_cursor()
Protocol_text::~Protocol_text()
Item_param::basic_const_item() const
handler::primary_key_is_clustered()
SEL_ARG::find_range(SEL_ARG*)
SEL_TREE::SEL_TREE()
Query_arena::~Query_arena()
Item_equal_iterator::operator++(int)
List_iterator<Item_field>::operator++(int)
List<Item_equal>::List()
bitmap_union
st_table_share::get_table_ref_version() const
st_select_lex::next_select()
Field::charset() const
Item_bool_rowready_func2::Item_bool_rowready_func2(Item*, Item*)
setup_copy_fields(THD*, TMP_TABLE_PARAM*, Item**, List<Item>&, List<Item>&, unsigned int, List<Item>&)
List_iterator_fast<Item_field>::operator++(int)
Field::pack_length() const
SEL_ARG::first()
init_block
Field_str::max_display_length()
optimize_keyuse(JOIN*, st_dynamic_array*)
greedy_search(JOIN*, unsigned long long, unsigned int, unsigned int)
net_send_ok(THD*, unsigned int, unsigned int, unsigned long, unsigned long long, char const*)
TABLE_LIST::is_anonymous_derived_table() const
handler::is_fatal_error(int, unsigned int)
Item_int::type() const
list_node::list_node(void*, list_node*)
TRP_RANGE::make_quick(PARAM*, bool, st_mem_root*)
add_group_and_distinct_keys(JOIN*, st_join_table*)
Query_arena::is_conventional() const
check_table_name(char const*, unsigned int, bool)
table_cache_key
QUICK_SELECT_I::~QUICK_SELECT_I()
st_select_lex::init_query()
String::set_charset(charset_info_st*)
ha_myisam::extra(ha_extra_function)
ha_heap::rnd_next(unsigned char*)
Discrete_intervals_list::empty_no_free()
cmp_item::cmp_item()
cleanup_dirname
heap_position
decimal2string
Item_bool_func2::~Item_bool_func2()
st_select_lex::non_agg_field_used() const
Item_int_func::Item_int_func(Item*, Item*)
Arg_comparator::compare()
Item_field::equal_fields_propagator(unsigned char*)
Item_func::walk(bool (Item::*)(unsigned char*), bool, unsigned char*)
List_iterator<Item_func_match>::List_iterator(List<Item_func_match>&)
List<Item_field>::List()
mysql_lock_tables(THD*, st_table**, unsigned int, unsigned int, bool*)
Field::reset_fields()
List_iterator_fast<TABLE_LIST>::List_iterator_fast(List<TABLE_LIST>&)
lex_casecmp(char const*, char const*, unsigned int)
Bitmap<64u>::is_prefix(unsigned int) const
Field::real_maybe_null()
handler::cond_push(Item const*)
QUICK_RANGE_SELECT::get_type()
find_keyword(Lex_input_stream*, unsigned int, bool)
QUICK_RANGE::QUICK_RANGE(unsigned char const*, unsigned int, unsigned long, unsigned char const*, unsigned int, unsigned long, unsigned int)
restore_prev_nj_state(st_join_table*)
Item::replace_equal_field(unsigned char*)
close_thread_table(THD*, st_table**)
List_iterator_fast<Item_equal>::List_iterator_fast(List<Item_equal>&)
SEL_ARG::cmp_min_to_min(SEL_ARG*)
list_node::~list_node()
strmake
SEL_ARG::free_tree()
Copy_field::get_copy_func(Field*, Field*)
handler::ha_open(st_table*, char const*, int, int)
my_tmpdir
create_tmp_field(THD*, st_table*, Item*, Item::Type, Item***, Field**, Field**, bool, bool, bool, bool, unsigned int)
JOIN::make_simple_join(JOIN*, st_table*)
SEL_ARG::simple_key()
decide_logging_format(THD*, TABLE_LIST*)
get_best_ror_intersect(PARAM const*, SEL_TREE*, double, bool*)
handler::ha_index_init(unsigned int, bool)
Field_long::max_display_length()
JOIN::save_join_tab()
handler::index_end()
pick_table_access_method(st_join_table*)
Item_bool_func::~Item_bool_func()
base_list_iterator::ref()
THD::current_utime()
handler::read_time(unsigned int, unsigned int, unsigned long)
Field::is_real_null(long long)
ha_heap::position(unsigned char const*)
Query_arena::memdup(void const*, unsigned long)
SEL_ARG::clone_and(SEL_ARG*)
handler::ha_external_lock(THD*, int)
open_cached_file
Field_num::size_of() const
SEL_ARG::store_max(unsigned int, unsigned char**, unsigned int)
decimal_round
Item::walk(bool (Item::*)(unsigned char*), bool, unsigned char*)
heap_extra
List_iterator<Cached_item>::operator++(int)
get_statement_id_as_hash_key
Protocol::storage_packet()
List<Item_field>::head()
handler::ha_rnd_init(bool)
my_hash_search
ha_myisam::external_lock(THD*, int)
create_distinct_group(THD*, Item**, st_order*, List<Item>&, List<Item>&, bool*)
Query_arena::Query_arena()
hp_get_new_block
List<Item_field>::~List()
mysql_unlock_tables(THD*, st_mysql_lock*)
_my_strdup
st_lex::set_trg_event_type_for_tables()
Field::sort_length() const
Item_basic_constant::used_tables() const
SEL_ARG::SEL_ARG(Field*, unsigned char, unsigned char*, unsigned char*, unsigned char, unsigned char, unsigned char)
fn_format
ha_heap::extra(ha_extra_function)
reap_plugins()
st_table::prepare_for_position()
list_contains_unique_index(st_table*, bool (*)(Field*, void*), void*)
Item_func::Item_func()
QUICK_RANGE_SELECT::range_end()
List_iterator<Item>::ref()
handler::ha_index_end()
List<Item>::head()
List_iterator_fast<TABLE_LIST>::operator++(int)
Item_field::eq(Item const*, bool) const
my_decimal::~my_decimal()
base_list_iterator::rewind()
Field::get_key_image(unsigned char*, unsigned int, Field::imagetype)
List<Item_equal>::~List()
Field::cmp_type() const
st_select_lex::set_non_agg_field_used(bool)
Item_bool_func::Item_bool_func()
Bitmap<64u>::to_ulonglong() const
st_select_lex_node::init_query()
List<SEL_IMERGE>::List()
remove_leading_zeroes
Item::subst_argument_checker(unsigned char**)
convert_dirname
ha_heap::info(unsigned int)
tmp_use_all_columns(st_table*, st_bitmap*)
Field::key_cmp(unsigned char const*, unsigned char const*)
Item_int::Item_int(unsigned long long, unsigned int)
bitmap_set_bit
Query_tables_list::reset_query_tables_list(bool)
ha_myisam::records_in_range(unsigned int, st_key_range*, st_key_range*)
handler::ha_rnd_end()
ha_heap::open(char const*, int, unsigned int)
QUICK_RANGE_SELECT::init()
parse_sql(THD*, Parser_state*, Object_creation_ctx*)
Item_sum::type() const
Field_string::new_field(st_mem_root*, st_table*, bool)
Item::send(Protocol*, String*)
change_to_use_tmp_fields(THD*, Item**, List<Item>&, List<Item>&, unsigned int, List<Item>&)
Protocol::prepare_for_send(List<Item>*)
Field::eq_def(Field*)
my_eof(THD*)
my_string_ptr_sort
handler::index_init(unsigned int, bool)
Field::get_image(unsigned char*, unsigned int, charset_info_st*)
SEL_ARG::increment_use_count(long)
bitmap_lock
end_active_trans(THD*)
ha_commit_trans(THD*, bool)
Item_field::find_item_equal(COND_EQUAL*)
Field::binary() const
Lex_input_stream::start_token()
Query_arena::~Query_arena()
intern_plugin_lock(st_lex*, st_plugin_int**, char const*, unsigned int)
optimizer_flag(THD*, unsigned int)
create_tmp_field_from_field(THD*, Field*, char const*, st_table*, Item_field*, unsigned int)
setup_sum_funcs(THD*, Item_sum**)
Lex_input_stream::yyUnget()
base_list_iterator::remove()
thr_multi_unlock
SEL_ARG::cmp_max_to_max(SEL_ARG*)
Item_sum::update_used_tables()
end_io_cache
st_table::needs_reopen_or_name_lock()
Field::key_length() const
I_List<Item_change_record>::empty()
THD::end_statement()
Bitmap<64u>::length() const
Item_int_func::Item_int_func()
Item_sum_sum::fix_length_and_dec()
mysql_lock_tables_check(THD*, st_table**, unsigned int, unsigned int)
Item_field::subst_argument_checker(unsigned char**)
Bitmap<64u>::operator==(Bitmap<64u> const&) const
st_select_lex_unit::init_query()
thr_unlock
Item_sum::check_sum_func(THD*, Item**)
my_hash_key
List_iterator_fast<Item_field>::List_iterator_fast(List<Item_field>&)
Field_long::pack_length() const
sql_memdup(void const*, unsigned long)
sel_add(SEL_ARG*, SEL_ARG*)
st_select_lex_unit::set_thd(THD*)
thr_lock
find_field_in_item_list(Field*, void*)
Item_equal::select_optimize() const
List<Item_equal>::pop()
ha_myisam::store_lock(THD*, st_thr_lock_data**, thr_lock_type)
plugin_unlock_list(THD*, st_plugin_int***, unsigned int)
QUICK_SELECT_I::QUICK_SELECT_I()
close_open_tables(THD*)
QUICK_SELECT_I::clustered_pk_range()
dirname_length
unlock_locked_tables(THD*)
unlock_external(THD*, st_table**, unsigned int)
Copy_field::set(Field*, Field*, bool)
lex_end(st_lex*)
lock_external(THD*, st_table**, unsigned int)
Item::equal_fields_propagator(unsigned char*)
List<Item_field>::push_back(Item_field*)
bitmap_is_set
Item::field_type() const
add_found_match_trig_cond(st_join_table*, Item*, st_join_table*)
get_range(SEL_ARG**, SEL_ARG**, SEL_ARG*)
add_key_equal_fields(key_field_t**, unsigned int, Item_func*, Item_field*, bool, Item**, unsigned int, unsigned long long, st_sargable_param**)
store_key_item::~store_key_item()
I_List_iterator<Item_change_record>::operator++(int)
heap_open_from_share
handler::estimate_rows_upper_bound()
hp_close
Item_func_opt_neg::subst_argument_checker(unsigned char**)
base_list::push_front(void*)
Diagnostics_area::set_ok_status(THD*, unsigned long, unsigned long long, char const*)
my_hash_next
Item_func_between::select_optimize() const
const_expression_in_where(Item*, Item*, Item**)
handler::rnd_end()
select_result::set_thd(THD*)
Field_string::real_type() const
Field::sort_charset() const
thr_multi_lock
Protocol_text::prepare_for_resend()
List_iterator<Item_func_match>::operator++(int)
hp_clear_keys
handler::handler(handlerton*, st_table_share*)
Lex_input_stream::Lex_input_stream()
handler::ha_drop_table(char const*)
st_decimal_t::st_decimal_t()
has_write_table_auto_increment_not_first_in_pk(TABLE_LIST*)
Item::Item(THD*, Item*)
Lex_input_stream::yyPeek()
ha_heap::drop_table(char const*)
init_tmp_table_share(THD*, st_table_share*, char const*, unsigned int, char const*, char const*)
safe_mutex_destroy
ha_heap::update_key_stats()
Lex_input_stream::yySkip()
SEL_ARG::is_null_interval()
st_table_share::db_type() const
hp_free_level
QUICK_RANGE::make_max_endpoint(st_key_range*)
free_field_buffers_larger_than(st_table*, unsigned int)
ha_lock_engine(THD*, handlerton const*)
unpack_dirname
handler::init()
Lex_input_stream::init(THD*, char*, unsigned int)
intern_plugin_unlock(st_lex*, st_plugin_int**)
List_iterator<Item_func_set_user_var>::List_iterator(List<Item_func_set_user_var>&)
init_functions
my_decimal2string(unsigned int, my_decimal const*, unsigned int, unsigned int, char, String*)
my_ismbchar_utf8
Item_sum::depended_from()
JOIN::set_items_ref_array(Item**)
get_new_handler(st_table_share*, st_mem_root*, handlerton*)
tmp_restore_column_map(st_bitmap*, unsigned int*)
mi_restore_status
Item::quick_fix_field()
strlength
SEL_ARG::cmp_max_to_min(SEL_ARG*)
heap_info
SQL_I_List<st_order>::empty()
Copy_field::~Copy_field()
Item_field::collect_item_field_processor(unsigned char*)
Lex_input_stream::body_utf8_append(char const*, char const*)
ha_heap::set_keys_for_scanning()
List_iterator_fast<Item>::sublist(List<Item>&, unsigned int)
my_hash_mask
bitmap_set_bit
ha_heap::close()
Item::init_make_field(Send_field*, enum_field_types)
hp_free
Item_sum::cleanup()
rr_handle_error(READ_RECORD*, int)
st_select_lex_unit::unclean()
ha_myisam::rnd_init(bool)
handler::lock_count() const
list_delete
Item_sum::init_sum_func_check(THD*)
Item_sum_sum::result_type() const
List_iterator_fast<Item_equal>::operator++(int)
SQL_I_List<TABLE_LIST>::empty()
Item_sum::reset()
TRP_RANGE::TRP_RANGE(SEL_ARG*, unsigned int)
list_add
store_key::~store_key()
heap_create_handler(handlerton*, st_table_share*, st_mem_root*)
List_iterator<Item_func_set_user_var>::operator++(int)
my_decimal_string_length(my_decimal const*)
dbug_tmp_restore_column_map(st_bitmap*, unsigned int*)
Field_string::reset()
THD::st_transactions::cleanup()
hp_clear
simple_open_n_lock_tables(THD*, TABLE_LIST*)
Lex_input_stream::yyLength()
Parser_state::~Parser_state()
open_tmp_table(st_table*)
Item_field::register_field_in_read_map(unsigned char*)
Item_equal::members()
Field_string::max_packed_col_length(unsigned int)
Item_ident::Item_ident(THD*, Item_ident*)
Sql_alloc::operator new(unsigned long, st_mem_root*)
bitmap_get_first
get_first_not_set
List_iterator<Item_field>::rewind()
handler::~handler()
is_log_table_write_query(enum_sql_command)
Item_field::~Item_field()
bitmap_lock_clear_bit
TABLE_LIST::containing_subselect()
Item_sum_sum::clear()
ha_heap::rnd_init(bool)
Field::init(st_table*)
base_list_iterator::sublist(base_list&, unsigned int)
plugin_lock(THD*, st_plugin_int***, char const*, unsigned int)
Copy_field::Copy_field()
Item_field::Item_field(THD*, Item_field*)
THD::binlog_flush_pending_rows_event(bool)
ha_heap::table_flags() const
Item::val_string_from_decimal(String*)
Item::decimal_precision() const
Item_field::get_tmp_table_field()
Item_sum::const_item() const
plugin_unlock(THD*, st_plugin_int**)
setup_io_cache
TABLE_READ_PLAN::operator new(unsigned long, st_mem_root*)
st_table::is_children_attached()
tmp_restore_column_map(st_bitmap*, unsigned int*)
normalize_dirname
Diagnostics_area::message() const
get_ptr_compare
Yacc_state::~Yacc_state()
Lex_input_stream::set_echo(bool)
SQL_I_List<TABLE_LIST>::link_in_list(TABLE_LIST*, TABLE_LIST**)
dirname_part
my_fill_8bit
xid_t::null()
Item_field::get_tmp_table_item(THD*)
Diagnostics_area::last_insert_id() const
Item::float_length(unsigned int) const
Table_ident::Table_ident(st_mysql_lex_string)
st_ha_create_information::st_ha_create_information()
List_iterator<Item_field>::remove()
init_sum_functions(Item_sum**, Item_sum**)
Bitmap<64u>::Bitmap(unsigned int)
Item_sum::get_arg_count()
Lex_input_stream::get_ptr()
next_query_id()
st_select_lex::get_table_list()
Field_string::key_type() const
ftparser_call_deinitializer
thr_lock_delete
thr_lock_init
wake_up_waiters
Lex_input_stream::restart_token()
List<Item>::push_back(Item*)
my_decimal_set_zero(my_decimal*)
st_parsing_options::reset()
ha_myisam::table_flags() const
JOIN::init_items_ref_array()
Sql_alloc::operator new[](unsigned long)
Lex_input_stream::eof()
bitmap_lock_set_next
my_ok(THD*, unsigned long, unsigned long long, char const*)
Item::make_field(Send_field*)
Item_ident::~Item_ident()
SQL_I_List<Sroutine_hash_entry>::empty()
TABLE_READ_PLAN::TABLE_READ_PLAN()
st_lex::copy_db_to(char**, unsigned long*) const
Field_str::binary() const
Parser_state::Parser_state()
strmake_root
st_ha_create_information::~st_ha_create_information()
Field::free()
dbug_tmp_restore_column_map(st_bitmap*, unsigned int*)
my_decimal_precision_to_length_no_truncation(unsigned int, unsigned char, bool)
dbug_tmp_use_all_columns(st_table*, st_bitmap*)
safe_mutex_init
Field::move_field_offset(long long)
ha_heap::~ha_heap()
xid_t::get_my_xid()
Item_sum_sum::val_str(String*)
ha_statistics::ha_statistics()
DTCollation::set(Derivation)
Lex_input_stream::get_tok_start()
bitmap_unlock
Field_str::result_type() const
setup_tmp_table_column_bitmaps(st_table*, unsigned char*)
is_schema_db(char const*, unsigned long)
ha_heap::ha_heap(handlerton*, st_table_share*)
bitmap_set_bit
Lex_input_stream::~Lex_input_stream()
Field::move_field(unsigned char*, unsigned char*, unsigned char)
bitmap_clear_bit
char_array_size(unsigned int, unsigned int)
mi_get_status
String::String(String const&)
Discrete_interval::replace(unsigned long long, unsigned long long, unsigned long long)
Item::register_field_in_read_map(unsigned char*)
bitmap_set_next
Query_tables_list::add_to_query_tables(TABLE_LIST*)
SQL_I_List<TABLE_LIST>::SQL_I_List()
intern_filename
Diagnostics_area::affected_rows() const
Table_ident::is_derived_table() const
st_lex::push_context(Name_resolution_context*)
base_list::concat(base_list*)
st_select_lex::init_order()
Discrete_interval::Discrete_interval()
Item_sum_sum::val_decimal(my_decimal*)
Lex_input_stream::body_utf8_append(char const*)
Sql_alloc::operator new[](unsigned long, st_mem_root*)
Bitmap<64u>::set_all()
Item_sum::used_tables() const
List<Item>::concat(List<Item>*)
make_group_fields(JOIN*, JOIN*)
my_decimal_round(unsigned int, my_decimal const*, int, bool, my_decimal*)
Field_string::size_of() const
Field::max_packed_col_length(unsigned int)
alloc_group_fields(JOIN*, st_order*)
get_token(Lex_input_stream*, unsigned int, unsigned int)
Item_sum_sum::sum_func() const
st_select_lex::set_agg_func_used(bool)
SQL_I_List<TABLE_LIST>::~SQL_I_List()
Item::is_expensive_processor(unsigned char*)
bitmap_set_bit
Name_resolution_context::init()
heap_scan_init
my_decimal_length_to_precision(unsigned int, unsigned int, bool)
THD::copy_db_to(char**, unsigned long*)
my_hash_rec_mask
Sql_alloc::operator delete[](void*, unsigned long)
TABLE_LIST::set_table_ref_id(st_table_share*)
vio_description
Item_sum::get_arg(int)
List<Name_resolution_context>::push_front(Name_resolution_context*)
List_iterator<Item>::rewind()
List_iterator<Item_field>::ref()
Query_arena::strmake(char const*, unsigned long)
THD::set_time_after_lock()
thr_lock_data_init
Item_sum::setup(THD*)
system_filename
Yacc_state::Yacc_state()
handler::extra_opt(ha_extra_function, unsigned long)
Send_field::Send_field()
Lex_input_stream::body_utf8_append_literal(THD*, st_mysql_lex_string const*, charset_info_st*, char const*)
Parser_state::init(THD*, char*, unsigned int)
Lex_input_stream::get_cpp_tok_start()
yydestruct(char const*, int, YYSTYPE*)
handler::unlock_row()
Field::offset(unsigned char*)
st_table_share::require_write_privileges()
read_block
st_select_lex_node::init_select()
Field_string::has_charset() const
link_to_file_list
String::set_or_copy_aligned(char const*, unsigned int, charset_info_st*)
check_prepared_statement(Prepared_statement*)
link_changed
find_field_in_table_ref(THD*, TABLE_LIST*, char const*, unsigned int, char const*, char const*, char const*, Item**, bool, bool, unsigned int*, bool, TABLE_LIST**)
my_hash_insert
plugin_thdvar_cleanup(THD*)
Ha_trx_info::reset()
Item::operator new(unsigned long, st_mem_root*)
Item_ident::Item_ident(Name_resolution_context*, char const*, char const*, char const*)
Item_param::Item_param(unsigned int)
Item_sum_sum::~Item_sum_sum()
List<List<Item>>::~List()
Prepared_statement::prepare(char const*, unsigned int)
Prepared_statement::~Prepared_statement()
Protocol_binary::Protocol_binary(THD*)
Query_arena::set_query_arena(Query_arena*)
Query_tables_list::Query_tables_list()
SQL_I_List<Item_trigger_field>::~SQL_I_List()
SQL_I_List<Sroutine_hash_entry>::SQL_I_List()
Statement::~Statement()
base_ilist::append(ilink*)
check_key_in_view(THD*, TABLE_LIST*)
find_files(THD*, List<st_mysql_lex_string>*, char const*, char const*, char const*, bool)
get_length_encoded_string(char**, unsigned long*, unsigned long*)
init_param_array(Prepared_statement*)
mi_get_pointer_length
mi_keyseg_read
my_hash_free_elements
my_mb_wc_latin1
my_message_sql
mysql_test_select(Prepared_statement*, TABLE_LIST*)
net_send_error(THD*, unsigned int, char const*)
plugin_foreach_with_mask(THD*, char (*)(THD*, st_plugin_int**, void*), int, unsigned int, void*)
sp_cache_version(sp_cache**)
st_lex::~st_lex()
st_select_lex::add_joined_table(TABLE_LIST*)
st_select_lex::set_lock_for_tables(thr_lock_type)
st_select_lex::st_select_lex()
st_select_lex_unit::prepare(THD*, select_result*, unsigned long)
st_select_lex_unit::st_select_lex_unit()
thr_lock_info_init
unique_table(THD*, TABLE_LIST*, TABLE_LIST*, bool)
vio_init
