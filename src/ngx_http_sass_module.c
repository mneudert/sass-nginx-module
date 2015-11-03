#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <sass.h> 

// required for libsass < 3.3.0
#ifndef SASS_C_CONTEXT_H
#include <sass_context.h>
#endif

typedef struct {
    ngx_flag_t  enable;
    ngx_uint_t  error_log;
    ngx_str_t   include_path;
    ngx_uint_t  output_style;
    ngx_flag_t  source_comments;
    ngx_uint_t  precision;
    ngx_flag_t  map_embed;
    ngx_flag_t  source_type;
    ngx_flag_t  omit_url;
    ngx_flag_t  source_map_contents;
    ngx_str_t   source_map_root;
    ngx_str_t   source_map_file;

} ngx_http_sass_loc_conf_t;


static ngx_int_t ngx_http_sass_init(ngx_conf_t *cf);
static void *ngx_http_sass_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_sass_merge_loc_conf(ngx_conf_t *cf,void *parent, void *child);
static char *ngx_http_sass_error_log_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_sass_output_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t  ngx_http_sass_commands[] = {
    { ngx_string("sass_comments"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, source_comments),
      NULL },

    { ngx_string("sass_compile"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, enable),
      NULL },

    { ngx_string("sass_error_log"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_http_sass_error_log_value,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, error_log),
      NULL },

    { ngx_string("sass_include_path"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, include_path),
      NULL },

     { ngx_string("sass_precision"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, precision),
      NULL },

    { ngx_string("sass_source_type"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, source_type),
      NULL },

    { ngx_string("sass_map_file"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, source_map_file),
      NULL },

    { ngx_string("sass_map_root"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, source_map_root),
      NULL },

    { ngx_string("sass_map_embed"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, map_embed),
      NULL },

     { ngx_string("sass_map_url"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, omit_url),
      NULL },

    { ngx_string("sass_output"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_http_sass_output_value,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, output_style),
      NULL },

    ngx_null_command
};


static ngx_str_t  ngx_http_sass_type = ngx_string("text/css");
static ngx_str_t  ngx_http_map_type = ngx_string("application/octet-stream");


static ngx_str_t  err_levels[] = {
    ngx_null_string,
    ngx_string("emerg"),
    ngx_string("alert"),
    ngx_string("crit"),
    ngx_string("error"),
    ngx_string("warn"),
    ngx_string("notice"),
    ngx_string("info"),
    ngx_string("debug")
};


static ngx_http_module_t  ngx_http_sass_module_ctx = {
    NULL,                          /* preconfiguration */
    ngx_http_sass_init,            /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_http_sass_create_loc_conf, /* create location configuration */
    ngx_http_sass_merge_loc_conf   /* merge location configuration */
};


ngx_module_t  ngx_http_sass_module = {
    NGX_MODULE_V1,
    &ngx_http_sass_module_ctx, /* module context */
    ngx_http_sass_commands,    /* module directives */
    NGX_HTTP_MODULE,           /* module type */

    NULL,                      /* init master */
    NULL,                      /* init module */
    NULL,                      /* init process */
    NULL,                      /* init thread */

    NULL,                      /* exit thread */
    NULL,                      /* exit process */
    NULL,                      /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_sass_handler(ngx_http_request_t *r)
{
    size_t        root;
    u_char       *last;
    ngx_buf_t*    b;
    ngx_chain_t   out;
    ngx_str_t     content, path;

    ngx_http_sass_loc_conf_t  *clcf;

    const char                *output;
    const char                *mapfile;
    struct Sass_Context       *ctx;
    struct Sass_File_Context  *ctx_file;
    struct Sass_Options       *options;

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_DECLINED;
    }

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_sass_module);

    if (!clcf->enable) {
        return NGX_DECLINED;
    }

    last = ngx_http_map_uri_to_path(r, &path, &root, 0);

    if (NULL == last) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    path.len = last - path.data;

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "sass compile file: \"%V\"", &path);

    ctx_file = sass_make_file_context((char *) path.data);
    ctx      = sass_file_context_get_context(ctx_file);
    options  = sass_file_context_get_options(ctx_file);

    sass_option_set_precision(options, (int) clcf->precision);
    sass_option_set_source_comments(options, clcf->source_comments);
    sass_option_set_include_path(options, (char *) clcf->include_path.data);

    sass_option_set_is_indented_syntax_src(options, clcf->source_type);
    sass_option_set_omit_source_map_url(options, clcf->omit_url);
    sass_option_set_source_map_embed(options, clcf->map_embed);
    sass_option_set_source_map_contents(options, clcf->source_map_contents);

    sass_option_set_input_path(options, (char *) path.data);
    sass_option_set_output_style(options, clcf->output_style);

    if (clcf->source_map_file.len > 0) {
    sass_option_set_source_map_file(options,(char *) clcf->source_map_file.data);
    sass_option_set_omit_source_map_url(options, false);
    sass_option_set_source_map_contents(options, true);
    }

    if (clcf->source_map_root.len > 0) {
    sass_option_set_source_map_root(options, (char *) clcf->source_map_root.data);
    }

      if (sass_context_get_error_status(ctx)
        && sass_context_get_error_message(ctx)
    ) {
        ngx_log_error(clcf->error_log, r->connection->log, 0,
                      "sass compilation error: %s",
                      sass_context_get_error_message(ctx));
        sass_delete_file_context(ctx_file);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    } else if (sass_context_get_error_status(ctx)) {
        ngx_log_error(clcf->error_log, r->connection->log, 0,
                      "sass compilation error");
        sass_delete_file_context(ctx_file);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    if (NULL == b) {
        ngx_log_error(clcf->error_log, r->connection->log, 0,
                      "sass compilation error: %s",
                      sass_context_get_error_message(ctx));
        sass_delete_file_context(ctx_file);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (clcf->source_map_file.len > 0) {

    } else {

    output = sass_context_get_output_string(ctx);

    out.buf  = b;
    out.next = NULL;

    content.len  = sizeof(output) - 1;
    content.data = ngx_pnalloc(r->pool, strlen(output));

    if (NULL == content.data) {
        ngx_log_error(clcf->error_log, r->connection->log, 0,
                      "sass failed to allocate response buffer");
        sass_delete_file_context(ctx_file);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_cpystrn(content.data, (unsigned char *) output, strlen(output));

    b->start    = b->pos = content.data;
    b->last     = b->end = content.data + strlen(output);
    b->memory   = 1;
    if (clcf->source_map_file.len > 0) {
    b->last_buf = 0;
    } else {
    b->last_buf = 1;
    }

    r->headers_out.status           = NGX_HTTP_OK;
    r->headers_out.content_type     = ngx_http_sass_type;
    r->headers_out.content_length_n = strlen(output);
    }

    // Flush context if no map file
    if (clcf->source_map_file.len = 0) {
    sass_delete_file_context(ctx_file);
    }
    ngx_http_send_header(r);

    // End sequence if no map file
     if (!clcf->source_map_file) {
    ngx_http_output_filter(r, &out);
    } else {
    return ngx_http_output_filter(r, &out); 
    }
    
    mapfile = sass_context_get_source_map_string(ctx);

    out.buf  = b;
    out.next = NULL;

    content.len  = sizeof(mapfile) - 1;
    content.data = ngx_pnalloc(r->pool, strlen(mapfile));

    if (NULL == content.data) {
        ngx_log_error(clcf->error_log, r->connection->log, 0,
                      "sass failed to allocate response buffer");
        sass_delete_file_context(ctx_file);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_cpystrn(content.data, (unsigned char *) output, strlen(mapfile));

    b->start    = b->pos = content.data;
    b->last     = b->end = content.data + strlen(mapfile);
    b->memory   = 1;
    b->last_buf = 1;

    r->headers_out.status           = NGX_HTTP_OK;
    r->headers_out.content_type     = ngx_http_map_type;
    r->headers_out.content_length_n = strlen(mapfile);

    sass_delete_file_context(ctx_file);
    ngx_http_send_header(r);
    return ngx_http_output_filter(r, &out);
  
}

static void *
ngx_http_sass_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_sass_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_sass_loc_conf_t));

    if (NULL == conf) {
        return NULL;
    }

    conf->enable          = NGX_CONF_UNSET;
    conf->error_log       = NGX_LOG_ERR;
    conf->output_style    = SASS_STYLE_NESTED;
    conf->precision       = NGX_CONF_UNSET;
    conf->source_comments = NGX_CONF_UNSET;
    conf->omit_url          = NGX_CONF_UNSET;
    conf->source_type       = NGX_CONF_UNSET;
    conf->map_embed          = NGX_CONF_UNSET;
    conf->source_map_contents = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_http_sass_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_sass_loc_conf_t *prev = parent;
    ngx_http_sass_loc_conf_t *conf = child;

    ngx_conf_merge_off_value(conf->enable, prev->enable, 0);
    ngx_conf_merge_str_value(conf->include_path, prev->include_path, "");
    ngx_conf_merge_uint_value(conf->output_style, prev->output_style, SASS_STYLE_NESTED);
    ngx_conf_merge_uint_value(conf->precision, prev->precision, 5);
    ngx_conf_merge_off_value(conf->source_comments, prev->source_comments, 0);
    ngx_conf_merge_off_value(conf->omit_url, prev->omit_url, 0);
    ngx_conf_merge_off_value(conf->source_type, prev->source_type, 0);  
    ngx_conf_merge_off_value(conf->map_embed, prev->map_embed, 0);
    ngx_conf_merge_off_value(conf->source_map_contents, prev->source_map_contents, 0);
    ngx_conf_merge_str_value(conf->source_map_root, prev->source_map_root, "");
    ngx_conf_merge_str_value(conf->source_map_file, prev->source_map_file, "");

    return NGX_CONF_OK;
}


static char *
ngx_http_sass_error_log_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t  *value;
    ngx_uint_t  n;

    ngx_http_sass_loc_conf_t *slcf = conf;

    value = cf->args->elts;

    for (n = 1; n <= NGX_LOG_DEBUG; n++) {
        if (0 == ngx_strcmp(value[1].data, err_levels[n].data)) {
            slcf->error_log = n;
            return NGX_CONF_OK;
        }
    }

    ngx_conf_log_error(
        NGX_LOG_EMERG, cf, 0,
        "invalid sass_error_log parameter \"%V\"", &value[1]
    );

    return NGX_CONF_ERROR;
}


static char *
ngx_http_sass_output_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t  *value;

    ngx_http_sass_loc_conf_t *slcf = conf;

    value = cf->args->elts;

    if (0 == ngx_strcmp(value[1].data, "nested")) {
        slcf->output_style = SASS_STYLE_NESTED;
        return NGX_CONF_OK;
    }

    if (0 == ngx_strcmp(value[1].data, "expanded")) {
        slcf->output_style = SASS_STYLE_EXPANDED;
        return NGX_CONF_OK;
    }

    if (0 == ngx_strcmp(value[1].data, "compact")) {
        slcf->output_style = SASS_STYLE_COMPACT;
        return NGX_CONF_OK;
    }

    if (0 == ngx_strcmp(value[1].data, "compressed")) {
        slcf->output_style = SASS_STYLE_COMPRESSED;
        return NGX_CONF_OK;
    }

    ngx_conf_log_error(
        NGX_LOG_EMERG, cf, 0,
        "invalid sass_output parameter \"%V\"", &value[1]
    );

    return NGX_CONF_ERROR;
}


static ngx_int_t
ngx_http_sass_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt       *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    h    = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);

    if (NULL == h) {
        return NGX_ERROR;
    }

    *h = ngx_http_sass_handler;

    return NGX_OK;
}
