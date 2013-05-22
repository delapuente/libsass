#define SASS_PARSER

#include <unordered_map>

#ifndef SASS_PRELEXER
#include "prelexer.hpp"
#endif

#ifndef SASS_TOKEN
#include "token.hpp"
#endif

struct Selector_Lookahead {
  const char* found;
  bool has_interpolants;
};

namespace Sass {
  using std::string;
  using std::vector;
  using std::map;
  using namespace Prelexer;

  class Parser {
    string path;
    const char* source;
    const char* position;
    const char* end;
    size_t line;
    bool own_source;

    Context& context;

    AST_Node* root;
    Token lexed;

  private:
    // force the use of the "make_from_..." factory funtions
    Parser(Context& ctx);
  public:
    Parser(const Parser& p);
    ~Parser();

    static Parser make_from_file(Context& ctx, string path);
    static Parser make_from_source_chars(Context& ctx, const char* src, string path = "", bool own_source = false);
    static Parser make_from_token(Context& ctx, Token t, string path = "", size_t line_number = 1);

    template <prelexer mx>
    const char* peek(const char* start = 0)
    {
      if (!start) start = position;
      const char* after_whitespace;
      if (mx == block_comment) {
        after_whitespace = // start;
          zero_plus< alternatives<spaces, line_comment> >(start);
      }
      else if (/*mx == ancestor_of ||*/ mx == no_spaces) {
        after_whitespace = position;
      }
      else if (mx == spaces || mx == ancestor_of) {
        after_whitespace = mx(start);
        if (after_whitespace) {
          return after_whitespace;
        }
        else {
          return 0;
        }
      }
      else if (mx == optional_spaces) {
        after_whitespace = optional_spaces(start);
      }
      else {
        after_whitespace = spaces_and_comments(start);
      }
      const char* after_token = mx(after_whitespace);
      if (after_token) {
        return after_token;
      }
      else {
        return 0;
      }
    }

    template <prelexer mx>
    const char* lex()
    {
      const char* after_whitespace;
      if (mx == block_comment) {
        after_whitespace = // position;
          zero_plus< alternatives<spaces, line_comment> >(position);
      }
      else if (mx == ancestor_of || mx == no_spaces) {
        after_whitespace = position;
      }
      else if (mx == spaces) {
        after_whitespace = spaces(position);
        if (after_whitespace) {
          line += count_interval<'\n'>(position, after_whitespace);
          lexed = Token(position, after_whitespace);
          return position = after_whitespace;
        }
        else {
          return 0;
        }
      }
      else if (mx == optional_spaces) {
        after_whitespace = optional_spaces(position);
      }
      else {
        after_whitespace = spaces_and_comments(position);
      }
      const char* after_token = mx(after_whitespace);
      if (after_token) {
        line += count_interval<'\n'>(position, after_token);
        lexed = Token(after_whitespace, after_token);
        return position = after_token;
      }
      else {
        return 0;
      }
    }

    void read_bom();

    void parse_scss();
    AST_Node* parse_import();
    AST_Node* parse_include();
    AST_Node* parse_mixin_definition();
    AST_Node* parse_function_definition();
    AST_Node* parse_parameters();
    AST_Node* parse_parameter(AST_Node*::Type);
    AST_Node* parse_mixin_call(AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_arguments();
    AST_Node* parse_argument(AST_Node*::Type);
    AST_Node* parse_assignment();
    AST_Node* parse_propset();
    AST_Node* parse_ruleset(Selector_Lookahead lookahead, AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_selector_schema(const char* end_of_selector);
    AST_Node* parse_selector_group();
    AST_Node* parse_selector();
    AST_Node* parse_selector_combinator();
    AST_Node* parse_simple_selector_sequence();
    AST_Node* parse_simple_selector();
    AST_Node* parse_pseudo();
    AST_Node* parse_attribute_selector();
    AST_Node* parse_block(AST_Node* surrounding_ruleset, AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_rule();
    AST_Node* parse_values();
    AST_Node* parse_list();
    AST_Node* parse_comma_list();
    AST_Node* parse_space_list();
    AST_Node* parse_disjunction();
    AST_Node* parse_conjunction();
    AST_Node* parse_relation();
    AST_Node* parse_expression();
    AST_Node* parse_term();
    AST_Node* parse_factor();
    AST_Node* parse_value();
    AST_Node* parse_function_call();
    AST_Node* parse_string();
    AST_Node* parse_value_schema();
    AST_Node* parse_identifier_schema();
    AST_Node* parse_url_schema();
    AST_Node* parse_if_directive(AST_Node* surrounding_ruleset, AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_for_directive(AST_Node* surrounding_ruleset, AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_each_directive(AST_Node* surrounding_ruleset, AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_while_directive(AST_Node* surrounding_ruleset, AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_directive(AST_Node* surrounding_ruleset, AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_keyframes(AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_keyframe(AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_media_query(AST_Node*::Type inside_of = AST_Node*::none);
    AST_Node* parse_media_expression();
    AST_Node* parse_warning();

    Selector_Lookahead lookahead_for_selector(const char* start = 0);
    Selector_Lookahead lookahead_for_extension_target(const char* start = 0);

    void throw_syntax_error(string message, size_t ln = 0);
    void throw_read_error(string message, size_t ln = 0);
  };

  size_t check_bom_chars(const char* src, const unsigned char* bom, size_t len);
}