function Result$Err$0$(param0) {
  this._0 = param0;
}
Result$Err$0$.prototype.$tag = 0;
function Result$Ok$0$(param0) {
  this._0 = param0;
}
Result$Ok$0$.prototype.$tag = 1;
class $PanicError extends Error {}
function $panic() {
  throw new $PanicError();
}
function $bound_check(arr, index) {
  if (index < 0 || index >= arr.length) throw new Error("Index out of bounds");
}
function $compare_int(a, b) {
  return (a >= b) - (a <= b);
}
function Result$Err$1$(param0) {
  this._0 = param0;
}
Result$Err$1$.prototype.$tag = 0;
function Result$Ok$1$(param0) {
  this._0 = param0;
}
Result$Ok$1$.prototype.$tag = 1;
function Error$moonbitlang$47$x$47$fs$46$IOError$46$IOError(param0) {
  this._0 = param0;
}
Error$moonbitlang$47$x$47$fs$46$IOError$46$IOError.prototype.$tag = 2;
const Error$moonbitlang$47$core$47$builtin$46$CreatingViewError$46$IndexOutOfBounds = { $tag: 1 };
const Error$moonbitlang$47$core$47$builtin$46$CreatingViewError$46$InvalidIndex = { $tag: 0 };
const moonbitlang$core$builtin$$int_to_string_js = (x, radix) => {
  return x.toString(radix);
};
function $makebytes(a, b) {
  const arr = new Uint8Array(a);
  if (b !== 0) {
    arr.fill(b);
  }
  return arr;
}
function $make_array_len_and_init(a, b) {
  const arr = new Array(a);
  arr.fill(b);
  return arr;
}
const moonbitlang$core$builtin$$JSArray$push = (arr, val) => { arr.push(val); };
const $bytes_literal$0 = new Uint8Array();
const Option$None$2$ = { $tag: 0 };
function Option$Some$2$(param0) {
  this._0 = param0;
}
Option$Some$2$.prototype.$tag = 1;
const moonbitlang$x$fs$$read_file_ffi = function(path) {
   var fs = require('fs');
   try {
     const content = fs.readFileSync(path);
     globalThis.fileContent = content;
     return 0;
   } catch (error) {
     globalThis.errorMessage = error.message;
     return -1;
   }
 };
const moonbitlang$x$fs$$write_file_ffi = function(path, content) {
   var fs = require('fs');
   try {
     fs.writeFileSync(path, Buffer.from(content));
     return 0;
   } catch (error) {
     globalThis.errorMessage = error.message;
     return -1;
   }
 };
const moonbitlang$x$fs$$get_file_content_ffi = function() {
   return globalThis.fileContent;
 };
const moonbitlang$x$fs$$get_error_message_ffi = function() {
   return globalThis.errorMessage || '';
 };
function Result$Err$3$(param0) {
  this._0 = param0;
}
Result$Err$3$.prototype.$tag = 0;
function Result$Ok$3$(param0) {
  this._0 = param0;
}
Result$Ok$3$.prototype.$tag = 1;
function Result$Err$4$(param0) {
  this._0 = param0;
}
Result$Err$4$.prototype.$tag = 0;
function Result$Ok$4$(param0) {
  this._0 = param0;
}
Result$Ok$4$.prototype.$tag = 1;
function Result$Err$5$(param0) {
  this._0 = param0;
}
Result$Err$5$.prototype.$tag = 0;
function Result$Ok$5$(param0) {
  this._0 = param0;
}
Result$Ok$5$.prototype.$tag = 1;
const moonbitlang$x$fs$$path_exists_ffi = function(path) {
  var fs = require('fs');
  return fs.existsSync(path);
 };
const moonbitlang$x$fs$$create_dir_ffi = function(path) {
  var fs = require('fs');
  try {
    fs.mkdirSync(path, { recursive: true });
    return 0;
  } catch (error) {
    globalThis.errorMessage = error.message;
    return -1;
  }
 };
const moonbitlang$x$fs$$is_dir_ffi = function(path) {
  var fs = require('fs');
  try {
    const stats = fs.statSync(path);
    return stats.isDirectory() ? 1 : 0;
  } catch (error) {
    globalThis.errorMessage = error.message;
    return -1;
  }
 };
function Result$Err$6$(param0) {
  this._0 = param0;
}
Result$Err$6$.prototype.$tag = 0;
function Result$Ok$6$(param0) {
  this._0 = param0;
}
Result$Ok$6$.prototype.$tag = 1;
const moonbitlang$x$sys$internal$ffi$$get_cli_args_internal = function() {
  return process.argv.slice(1);
 };
const moonbitlang$x$sys$internal$ffi$$exit = function(code) {
   process.exit(code);
 };
function Result$Err$7$(param0) {
  this._0 = param0;
}
Result$Err$7$.prototype.$tag = 0;
function Result$Ok$7$(param0) {
  this._0 = param0;
}
Result$Ok$7$.prototype.$tag = 1;
function Result$Err$8$(param0) {
  this._0 = param0;
}
Result$Err$8$.prototype.$tag = 0;
function Result$Ok$8$(param0) {
  this._0 = param0;
}
Result$Ok$8$.prototype.$tag = 1;
const $$$64$ZSeanYves$47$Doclint$47$src$47$doclint_rules_thesis$46$PageLimitRule$36$as$36$64$ZSeanYves$47$Doclint$47$src$47$core$46$Rule = { method_0: ZSeanYves$Doclint$src$core$$Rule$id$0$, method_1: ZSeanYves$Doclint$src$core$$Rule$check$0$ };
const $$$64$ZSeanYves$47$Doclint$47$src$47$doclint_rules_thesis$46$RequireKeywordRule$36$as$36$64$ZSeanYves$47$Doclint$47$src$47$core$46$Rule = { method_0: ZSeanYves$Doclint$src$core$$Rule$id$1$, method_1: ZSeanYves$Doclint$src$core$$Rule$check$1$ };
const $$$64$moonbitlang$47$core$47$builtin$46$StringBuilder$36$as$36$64$moonbitlang$47$core$47$builtin$46$Logger = { method_0: moonbitlang$core$builtin$$Logger$write_string$2$, method_1: moonbitlang$core$builtin$$Logger$write_substring$3$, method_2: moonbitlang$core$builtin$$Logger$write_view$2$, method_3: moonbitlang$core$builtin$$Logger$write_char$2$ };
const ZSeanYves$Doclint$src$core$$issues_to_json$46$42$bind$124$202 = ",";
const ZSeanYves$Doclint$src$core$$parse_pages$46$pat$124$82 = "\"pages\"";
const ZSeanYves$Doclint$src$doclint_rules_thesis$$build_ruleset$46$record$47$591 = { min: 2, max: 50 };
const ZSeanYves$Doclint$src$doclint_rules_thesis$$build_ruleset$46$record$47$592 = { rid: "basic.require_abstract", keyword: "摘要", sev: 2 };
const ZSeanYves$Doclint$src$doclint_rules_thesis$$build_ruleset$46$record$47$593 = { page_limit: ZSeanYves$Doclint$src$doclint_rules_thesis$$build_ruleset$46$record$47$591, require_abs: ZSeanYves$Doclint$src$doclint_rules_thesis$$build_ruleset$46$record$47$592 };
const ZSeanYves$Doclint$src$doclint_rules_contract$$build_ruleset$46$record$47$595 = { min: 1, max: 200 };
const ZSeanYves$Doclint$src$doclint_rules_contract$$build_ruleset$46$record$47$596 = { rid: "contract.require_title", keyword: "合同", sev: 1 };
const ZSeanYves$Doclint$src$doclint_rules_contract$$build_ruleset$46$record$47$597 = { page_limit: ZSeanYves$Doclint$src$doclint_rules_contract$$build_ruleset$46$record$47$595, require_abs: ZSeanYves$Doclint$src$doclint_rules_contract$$build_ruleset$46$record$47$596 };
const ZSeanYves$Doclint$src$core$$parse_doc_json$46$constr$47$562 = new Result$Err$0$("bad json");
const moonbitlang$core$builtin$$brute_force_find$46$constr$47$212 = 0;
const moonbitlang$core$builtin$$boyer_moore_horspool_find$46$constr$47$198 = 0;
function moonbitlang$core$abort$$abort$4$(msg) {
  $panic();
}
function moonbitlang$core$abort$$abort$5$(msg) {
  return $panic();
}
function moonbitlang$core$builtin$$abort$4$(string, loc) {
  moonbitlang$core$abort$$abort$4$(`${string}\n  at ${moonbitlang$core$builtin$$Show$to_string$6$(loc)}\n`);
}
function moonbitlang$core$builtin$$abort$5$(string, loc) {
  return moonbitlang$core$abort$$abort$5$(`${string}\n  at ${moonbitlang$core$builtin$$Show$to_string$6$(loc)}\n`);
}
function moonbitlang$core$builtin$$StringBuilder$new$46$inner(size_hint) {
  return { val: "" };
}
function moonbitlang$core$builtin$$Logger$write_char$2$(self, ch) {
  const _bind = self;
  _bind.val = `${_bind.val}${String.fromCodePoint(ch)}`;
}
function moonbitlang$core$uint16$$UInt16$is_leading_surrogate(self) {
  return moonbitlang$core$builtin$$Compare$op_ge$7$(self, 55296) && moonbitlang$core$builtin$$Compare$op_le$7$(self, 56319);
}
function moonbitlang$core$uint16$$UInt16$is_trailing_surrogate(self) {
  return moonbitlang$core$builtin$$Compare$op_ge$7$(self, 56320) && moonbitlang$core$builtin$$Compare$op_le$7$(self, 57343);
}
function moonbitlang$core$builtin$$code_point_of_surrogate_pair(leading, trailing) {
  return (((Math.imul(leading - 55296 | 0, 1024) | 0) + trailing | 0) - 56320 | 0) + 65536 | 0;
}
function moonbitlang$core$array$$Array$at$8$(self, index) {
  const len = self.length;
  if (index >= 0 && index < len) {
    $bound_check(self, index);
    return self[index];
  } else {
    return $panic();
  }
}
function moonbitlang$core$builtin$$SourceLocRepr$parse(repr) {
  const _bind = { str: repr, start: 0, end: repr.length };
  const _data = _bind.str;
  const _start = _bind.start;
  const _end = _start + (_bind.end - _bind.start | 0) | 0;
  let _cursor = _start;
  let accept_state = -1;
  let match_end = -1;
  let match_tag_saver_0 = -1;
  let match_tag_saver_1 = -1;
  let match_tag_saver_2 = -1;
  let match_tag_saver_3 = -1;
  let match_tag_saver_4 = -1;
  let tag_0 = -1;
  let tag_1 = -1;
  let tag_1_1 = -1;
  let tag_1_2 = -1;
  let tag_3 = -1;
  let tag_2 = -1;
  let tag_2_1 = -1;
  let tag_4 = -1;
  _L: {
    let join_dispatch_19;
    _L$2: {
      if (_cursor < _end) {
        const _p = _cursor;
        const next_char = _data.charCodeAt(_p);
        _cursor = _cursor + 1 | 0;
        if (next_char < 65) {
          if (next_char < 64) {
            break _L;
          } else {
            while (true) {
              tag_0 = _cursor;
              if (_cursor < _end) {
                _L$3: {
                  const _p$2 = _cursor;
                  const next_char$2 = _data.charCodeAt(_p$2);
                  _cursor = _cursor + 1 | 0;
                  if (next_char$2 < 55296) {
                    if (next_char$2 < 58) {
                      break _L$3;
                    } else {
                      if (next_char$2 > 58) {
                        break _L$3;
                      } else {
                        if (_cursor < _end) {
                          _L$4: {
                            const _p$3 = _cursor;
                            const next_char$3 = _data.charCodeAt(_p$3);
                            _cursor = _cursor + 1 | 0;
                            if (next_char$3 < 56319) {
                              if (next_char$3 < 55296) {
                                break _L$4;
                              } else {
                                join_dispatch_19 = 7;
                                break _L$2;
                              }
                            } else {
                              if (next_char$3 > 56319) {
                                if (next_char$3 < 65536) {
                                  break _L$4;
                                } else {
                                  break _L;
                                }
                              } else {
                                join_dispatch_19 = 8;
                                break _L$2;
                              }
                            }
                          }
                          join_dispatch_19 = 0;
                          break _L$2;
                        } else {
                          break _L;
                        }
                      }
                    }
                  } else {
                    if (next_char$2 > 56318) {
                      if (next_char$2 < 57344) {
                        if (_cursor < _end) {
                          const _p$3 = _cursor;
                          const next_char$3 = _data.charCodeAt(_p$3);
                          _cursor = _cursor + 1 | 0;
                          if (next_char$3 < 56320) {
                            break _L;
                          } else {
                            if (next_char$3 > 57343) {
                              break _L;
                            } else {
                              continue;
                            }
                          }
                        } else {
                          break _L;
                        }
                      } else {
                        if (next_char$2 > 65535) {
                          break _L;
                        } else {
                          break _L$3;
                        }
                      }
                    } else {
                      if (_cursor < _end) {
                        const _p$3 = _cursor;
                        const next_char$3 = _data.charCodeAt(_p$3);
                        _cursor = _cursor + 1 | 0;
                        if (next_char$3 < 56320) {
                          break _L;
                        } else {
                          if (next_char$3 > 65535) {
                            break _L;
                          } else {
                            continue;
                          }
                        }
                      } else {
                        break _L;
                      }
                    }
                  }
                }
                continue;
              } else {
                break _L;
              }
            }
          }
        } else {
          break _L;
        }
      } else {
        break _L;
      }
    }
    let _tmp = join_dispatch_19;
    _L$3: while (true) {
      const dispatch_19 = _tmp;
      _L$4: {
        _L$5: {
          switch (dispatch_19) {
            case 3: {
              tag_1_2 = tag_1_1;
              tag_1_1 = tag_1;
              tag_1 = _cursor;
              if (_cursor < _end) {
                _L$6: {
                  const _p = _cursor;
                  const next_char = _data.charCodeAt(_p);
                  _cursor = _cursor + 1 | 0;
                  if (next_char < 55296) {
                    if (next_char < 58) {
                      if (next_char < 48) {
                        break _L$6;
                      } else {
                        tag_1 = _cursor;
                        tag_2_1 = tag_2;
                        tag_2 = _cursor;
                        tag_3 = _cursor;
                        if (_cursor < _end) {
                          _L$7: {
                            const _p$2 = _cursor;
                            const next_char$2 = _data.charCodeAt(_p$2);
                            _cursor = _cursor + 1 | 0;
                            if (next_char$2 < 59) {
                              if (next_char$2 < 46) {
                                if (next_char$2 < 45) {
                                  break _L$7;
                                } else {
                                  break _L$4;
                                }
                              } else {
                                if (next_char$2 > 47) {
                                  if (next_char$2 < 58) {
                                    _tmp = 6;
                                    continue _L$3;
                                  } else {
                                    _tmp = 3;
                                    continue _L$3;
                                  }
                                } else {
                                  break _L$7;
                                }
                              }
                            } else {
                              if (next_char$2 > 55295) {
                                if (next_char$2 < 57344) {
                                  if (next_char$2 < 56319) {
                                    _tmp = 7;
                                    continue _L$3;
                                  } else {
                                    _tmp = 8;
                                    continue _L$3;
                                  }
                                } else {
                                  if (next_char$2 > 65535) {
                                    break _L;
                                  } else {
                                    break _L$7;
                                  }
                                }
                              } else {
                                break _L$7;
                              }
                            }
                          }
                          _tmp = 0;
                          continue _L$3;
                        } else {
                          break _L;
                        }
                      }
                    } else {
                      if (next_char > 58) {
                        break _L$6;
                      } else {
                        _tmp = 1;
                        continue _L$3;
                      }
                    }
                  } else {
                    if (next_char > 56318) {
                      if (next_char < 57344) {
                        _tmp = 8;
                        continue _L$3;
                      } else {
                        if (next_char > 65535) {
                          break _L;
                        } else {
                          break _L$6;
                        }
                      }
                    } else {
                      _tmp = 7;
                      continue _L$3;
                    }
                  }
                }
                _tmp = 0;
                continue _L$3;
              } else {
                break _L;
              }
            }
            case 2: {
              tag_1 = _cursor;
              tag_2 = _cursor;
              if (_cursor < _end) {
                _L$6: {
                  const _p = _cursor;
                  const next_char = _data.charCodeAt(_p);
                  _cursor = _cursor + 1 | 0;
                  if (next_char < 55296) {
                    if (next_char < 58) {
                      if (next_char < 48) {
                        break _L$6;
                      } else {
                        _tmp = 2;
                        continue _L$3;
                      }
                    } else {
                      if (next_char > 58) {
                        break _L$6;
                      } else {
                        _tmp = 3;
                        continue _L$3;
                      }
                    }
                  } else {
                    if (next_char > 56318) {
                      if (next_char < 57344) {
                        _tmp = 8;
                        continue _L$3;
                      } else {
                        if (next_char > 65535) {
                          break _L;
                        } else {
                          break _L$6;
                        }
                      }
                    } else {
                      _tmp = 7;
                      continue _L$3;
                    }
                  }
                }
                _tmp = 0;
                continue _L$3;
              } else {
                break _L;
              }
            }
            case 0: {
              tag_1 = _cursor;
              if (_cursor < _end) {
                _L$6: {
                  const _p = _cursor;
                  const next_char = _data.charCodeAt(_p);
                  _cursor = _cursor + 1 | 0;
                  if (next_char < 55296) {
                    if (next_char < 58) {
                      break _L$6;
                    } else {
                      if (next_char > 58) {
                        break _L$6;
                      } else {
                        _tmp = 1;
                        continue _L$3;
                      }
                    }
                  } else {
                    if (next_char > 56318) {
                      if (next_char < 57344) {
                        _tmp = 8;
                        continue _L$3;
                      } else {
                        if (next_char > 65535) {
                          break _L;
                        } else {
                          break _L$6;
                        }
                      }
                    } else {
                      _tmp = 7;
                      continue _L$3;
                    }
                  }
                }
                _tmp = 0;
                continue _L$3;
              } else {
                break _L;
              }
            }
            case 8: {
              if (_cursor < _end) {
                const _p = _cursor;
                const next_char = _data.charCodeAt(_p);
                _cursor = _cursor + 1 | 0;
                if (next_char < 56320) {
                  break _L;
                } else {
                  if (next_char > 57343) {
                    break _L;
                  } else {
                    _tmp = 0;
                    continue _L$3;
                  }
                }
              } else {
                break _L;
              }
            }
            case 4: {
              tag_1 = _cursor;
              tag_4 = _cursor;
              if (_cursor < _end) {
                _L$6: {
                  const _p = _cursor;
                  const next_char = _data.charCodeAt(_p);
                  _cursor = _cursor + 1 | 0;
                  if (next_char < 55296) {
                    if (next_char < 58) {
                      if (next_char < 48) {
                        break _L$6;
                      } else {
                        _tmp = 4;
                        continue _L$3;
                      }
                    } else {
                      if (next_char > 58) {
                        break _L$6;
                      } else {
                        tag_1_2 = tag_1_1;
                        tag_1_1 = tag_1;
                        tag_1 = _cursor;
                        if (_cursor < _end) {
                          _L$7: {
                            const _p$2 = _cursor;
                            const next_char$2 = _data.charCodeAt(_p$2);
                            _cursor = _cursor + 1 | 0;
                            if (next_char$2 < 55296) {
                              if (next_char$2 < 58) {
                                if (next_char$2 < 48) {
                                  break _L$7;
                                } else {
                                  tag_1 = _cursor;
                                  tag_2_1 = tag_2;
                                  tag_2 = _cursor;
                                  if (_cursor < _end) {
                                    _L$8: {
                                      const _p$3 = _cursor;
                                      const next_char$3 = _data.charCodeAt(_p$3);
                                      _cursor = _cursor + 1 | 0;
                                      if (next_char$3 < 55296) {
                                        if (next_char$3 < 58) {
                                          if (next_char$3 < 48) {
                                            break _L$8;
                                          } else {
                                            _tmp = 5;
                                            continue _L$3;
                                          }
                                        } else {
                                          if (next_char$3 > 58) {
                                            break _L$8;
                                          } else {
                                            _tmp = 3;
                                            continue _L$3;
                                          }
                                        }
                                      } else {
                                        if (next_char$3 > 56318) {
                                          if (next_char$3 < 57344) {
                                            _tmp = 8;
                                            continue _L$3;
                                          } else {
                                            if (next_char$3 > 65535) {
                                              break _L;
                                            } else {
                                              break _L$8;
                                            }
                                          }
                                        } else {
                                          _tmp = 7;
                                          continue _L$3;
                                        }
                                      }
                                    }
                                    _tmp = 0;
                                    continue _L$3;
                                  } else {
                                    break _L$5;
                                  }
                                }
                              } else {
                                if (next_char$2 > 58) {
                                  break _L$7;
                                } else {
                                  _tmp = 1;
                                  continue _L$3;
                                }
                              }
                            } else {
                              if (next_char$2 > 56318) {
                                if (next_char$2 < 57344) {
                                  _tmp = 8;
                                  continue _L$3;
                                } else {
                                  if (next_char$2 > 65535) {
                                    break _L;
                                  } else {
                                    break _L$7;
                                  }
                                }
                              } else {
                                _tmp = 7;
                                continue _L$3;
                              }
                            }
                          }
                          _tmp = 0;
                          continue _L$3;
                        } else {
                          break _L;
                        }
                      }
                    }
                  } else {
                    if (next_char > 56318) {
                      if (next_char < 57344) {
                        _tmp = 8;
                        continue _L$3;
                      } else {
                        if (next_char > 65535) {
                          break _L;
                        } else {
                          break _L$6;
                        }
                      }
                    } else {
                      _tmp = 7;
                      continue _L$3;
                    }
                  }
                }
                _tmp = 0;
                continue _L$3;
              } else {
                break _L;
              }
            }
            case 5: {
              tag_1 = _cursor;
              tag_2 = _cursor;
              if (_cursor < _end) {
                _L$6: {
                  const _p = _cursor;
                  const next_char = _data.charCodeAt(_p);
                  _cursor = _cursor + 1 | 0;
                  if (next_char < 55296) {
                    if (next_char < 58) {
                      if (next_char < 48) {
                        break _L$6;
                      } else {
                        _tmp = 5;
                        continue _L$3;
                      }
                    } else {
                      if (next_char > 58) {
                        break _L$6;
                      } else {
                        _tmp = 3;
                        continue _L$3;
                      }
                    }
                  } else {
                    if (next_char > 56318) {
                      if (next_char < 57344) {
                        _tmp = 8;
                        continue _L$3;
                      } else {
                        if (next_char > 65535) {
                          break _L;
                        } else {
                          break _L$6;
                        }
                      }
                    } else {
                      _tmp = 7;
                      continue _L$3;
                    }
                  }
                }
                _tmp = 0;
                continue _L$3;
              } else {
                break _L$5;
              }
            }
            case 6: {
              tag_1 = _cursor;
              tag_2 = _cursor;
              tag_3 = _cursor;
              if (_cursor < _end) {
                _L$6: {
                  const _p = _cursor;
                  const next_char = _data.charCodeAt(_p);
                  _cursor = _cursor + 1 | 0;
                  if (next_char < 59) {
                    if (next_char < 46) {
                      if (next_char < 45) {
                        break _L$6;
                      } else {
                        break _L$4;
                      }
                    } else {
                      if (next_char > 47) {
                        if (next_char < 58) {
                          _tmp = 6;
                          continue _L$3;
                        } else {
                          _tmp = 3;
                          continue _L$3;
                        }
                      } else {
                        break _L$6;
                      }
                    }
                  } else {
                    if (next_char > 55295) {
                      if (next_char < 57344) {
                        if (next_char < 56319) {
                          _tmp = 7;
                          continue _L$3;
                        } else {
                          _tmp = 8;
                          continue _L$3;
                        }
                      } else {
                        if (next_char > 65535) {
                          break _L;
                        } else {
                          break _L$6;
                        }
                      }
                    } else {
                      break _L$6;
                    }
                  }
                }
                _tmp = 0;
                continue _L$3;
              } else {
                break _L;
              }
            }
            case 7: {
              if (_cursor < _end) {
                const _p = _cursor;
                const next_char = _data.charCodeAt(_p);
                _cursor = _cursor + 1 | 0;
                if (next_char < 56320) {
                  break _L;
                } else {
                  if (next_char > 65535) {
                    break _L;
                  } else {
                    _tmp = 0;
                    continue _L$3;
                  }
                }
              } else {
                break _L;
              }
            }
            case 1: {
              tag_1_1 = tag_1;
              tag_1 = _cursor;
              if (_cursor < _end) {
                _L$6: {
                  const _p = _cursor;
                  const next_char = _data.charCodeAt(_p);
                  _cursor = _cursor + 1 | 0;
                  if (next_char < 55296) {
                    if (next_char < 58) {
                      if (next_char < 48) {
                        break _L$6;
                      } else {
                        _tmp = 2;
                        continue _L$3;
                      }
                    } else {
                      if (next_char > 58) {
                        break _L$6;
                      } else {
                        _tmp = 1;
                        continue _L$3;
                      }
                    }
                  } else {
                    if (next_char > 56318) {
                      if (next_char < 57344) {
                        _tmp = 8;
                        continue _L$3;
                      } else {
                        if (next_char > 65535) {
                          break _L;
                        } else {
                          break _L$6;
                        }
                      }
                    } else {
                      _tmp = 7;
                      continue _L$3;
                    }
                  }
                }
                _tmp = 0;
                continue _L$3;
              } else {
                break _L;
              }
            }
            default: {
              break _L;
            }
          }
        }
        tag_1 = tag_1_2;
        tag_2 = tag_2_1;
        match_tag_saver_0 = tag_0;
        match_tag_saver_1 = tag_1;
        match_tag_saver_2 = tag_2;
        match_tag_saver_3 = tag_3;
        match_tag_saver_4 = tag_4;
        accept_state = 0;
        match_end = _cursor;
        break _L;
      }
      tag_1_1 = tag_1_2;
      tag_1 = _cursor;
      tag_2 = tag_2_1;
      if (_cursor < _end) {
        _L$5: {
          const _p = _cursor;
          const next_char = _data.charCodeAt(_p);
          _cursor = _cursor + 1 | 0;
          if (next_char < 55296) {
            if (next_char < 58) {
              if (next_char < 48) {
                break _L$5;
              } else {
                _tmp = 4;
                continue;
              }
            } else {
              if (next_char > 58) {
                break _L$5;
              } else {
                _tmp = 1;
                continue;
              }
            }
          } else {
            if (next_char > 56318) {
              if (next_char < 57344) {
                _tmp = 8;
                continue;
              } else {
                if (next_char > 65535) {
                  break _L;
                } else {
                  break _L$5;
                }
              }
            } else {
              _tmp = 7;
              continue;
            }
          }
        }
        _tmp = 0;
        continue;
      } else {
        break _L;
      }
    }
  }
  if (accept_state === 0) {
    let start_line;
    let _try_err;
    _L$2: {
      _L$3: {
        const _bind$2 = moonbitlang$core$string$$String$sub(_data, match_tag_saver_1 + 1 | 0, match_tag_saver_2);
        if (_bind$2.$tag === 1) {
          const _ok = _bind$2;
          start_line = _ok._0;
        } else {
          const _err = _bind$2;
          const _tmp = _err._0;
          _try_err = _tmp;
          break _L$3;
        }
        break _L$2;
      }
      start_line = $panic();
    }
    let start_column;
    let _try_err$2;
    _L$3: {
      _L$4: {
        const _bind$2 = moonbitlang$core$string$$String$sub(_data, match_tag_saver_2 + 1 | 0, match_tag_saver_3);
        if (_bind$2.$tag === 1) {
          const _ok = _bind$2;
          start_column = _ok._0;
        } else {
          const _err = _bind$2;
          const _tmp = _err._0;
          _try_err$2 = _tmp;
          break _L$4;
        }
        break _L$3;
      }
      start_column = $panic();
    }
    let pkg;
    let _try_err$3;
    _L$4: {
      _L$5: {
        const _bind$2 = moonbitlang$core$string$$String$sub(_data, _start + 1 | 0, match_tag_saver_0);
        if (_bind$2.$tag === 1) {
          const _ok = _bind$2;
          pkg = _ok._0;
        } else {
          const _err = _bind$2;
          const _tmp = _err._0;
          _try_err$3 = _tmp;
          break _L$5;
        }
        break _L$4;
      }
      pkg = $panic();
    }
    let filename;
    let _try_err$4;
    _L$5: {
      _L$6: {
        const _bind$2 = moonbitlang$core$string$$String$sub(_data, match_tag_saver_0 + 1 | 0, match_tag_saver_1);
        if (_bind$2.$tag === 1) {
          const _ok = _bind$2;
          filename = _ok._0;
        } else {
          const _err = _bind$2;
          const _tmp = _err._0;
          _try_err$4 = _tmp;
          break _L$6;
        }
        break _L$5;
      }
      filename = $panic();
    }
    let end_line;
    let _try_err$5;
    _L$6: {
      _L$7: {
        const _bind$2 = moonbitlang$core$string$$String$sub(_data, match_tag_saver_3 + 1 | 0, match_tag_saver_4);
        if (_bind$2.$tag === 1) {
          const _ok = _bind$2;
          end_line = _ok._0;
        } else {
          const _err = _bind$2;
          const _tmp = _err._0;
          _try_err$5 = _tmp;
          break _L$7;
        }
        break _L$6;
      }
      end_line = $panic();
    }
    let end_column;
    let _try_err$6;
    _L$7: {
      _L$8: {
        const _bind$2 = moonbitlang$core$string$$String$sub(_data, match_tag_saver_4 + 1 | 0, match_end);
        if (_bind$2.$tag === 1) {
          const _ok = _bind$2;
          end_column = _ok._0;
        } else {
          const _err = _bind$2;
          const _tmp = _err._0;
          _try_err$6 = _tmp;
          break _L$8;
        }
        break _L$7;
      }
      end_column = $panic();
    }
    return { pkg: pkg, filename: filename, start_line: start_line, start_column: start_column, end_line: end_line, end_column: end_column };
  } else {
    return $panic();
  }
}
function moonbitlang$core$builtin$$Logger$write_string$2$(self, str) {
  const _bind = self;
  _bind.val = `${_bind.val}${str}`;
}
function moonbitlang$core$builtin$$Compare$op_le$9$(x, y) {
  return moonbitlang$core$builtin$$Compare$compare$8$(x, y) <= 0;
}
function moonbitlang$core$builtin$$Compare$op_le$7$(x, y) {
  return $compare_int(x, y) <= 0;
}
function moonbitlang$core$builtin$$Compare$op_ge$9$(x, y) {
  return moonbitlang$core$builtin$$Compare$compare$8$(x, y) >= 0;
}
function moonbitlang$core$builtin$$Compare$op_ge$7$(x, y) {
  return $compare_int(x, y) >= 0;
}
function moonbitlang$core$string$$String$sub$46$inner(self, start, end) {
  const len = self.length;
  let end$2;
  if (end === undefined) {
    end$2 = len;
  } else {
    const _Some = end;
    const _end = _Some;
    end$2 = _end < 0 ? len + _end | 0 : _end;
  }
  const start$2 = start < 0 ? len + start | 0 : start;
  if (start$2 >= 0 && (start$2 <= end$2 && end$2 <= len)) {
    if (start$2 < len && moonbitlang$core$uint16$$UInt16$is_trailing_surrogate(self.charCodeAt(start$2))) {
      return new Result$Err$1$(Error$moonbitlang$47$core$47$builtin$46$CreatingViewError$46$InvalidIndex);
    }
    if (end$2 < len && moonbitlang$core$uint16$$UInt16$is_trailing_surrogate(self.charCodeAt(end$2))) {
      return new Result$Err$1$(Error$moonbitlang$47$core$47$builtin$46$CreatingViewError$46$InvalidIndex);
    }
    return new Result$Ok$1$({ str: self, start: start$2, end: end$2 });
  } else {
    return new Result$Err$1$(Error$moonbitlang$47$core$47$builtin$46$CreatingViewError$46$IndexOutOfBounds);
  }
}
function moonbitlang$core$string$$String$sub(self, start$46$opt, end) {
  let start;
  if (start$46$opt === undefined) {
    start = 0;
  } else {
    const _Some = start$46$opt;
    start = _Some;
  }
  return moonbitlang$core$string$$String$sub$46$inner(self, start, end);
}
function moonbitlang$core$builtin$$Logger$write_substring$3$(self, value, start, len) {
  let _tmp;
  let _try_err;
  _L: {
    _L$2: {
      const _bind = moonbitlang$core$string$$String$sub$46$inner(value, start, start + len | 0);
      if (_bind.$tag === 1) {
        const _ok = _bind;
        _tmp = _ok._0;
      } else {
        const _err = _bind;
        const _tmp$2 = _err._0;
        _try_err = _tmp$2;
        break _L$2;
      }
      break _L;
    }
    _tmp = $panic();
  }
  moonbitlang$core$builtin$$Logger$write_view$2$(self, _tmp);
}
function moonbitlang$core$builtin$$Show$to_string$6$(self) {
  const logger = moonbitlang$core$builtin$$StringBuilder$new$46$inner(0);
  moonbitlang$core$builtin$$Show$output$10$(self, { self: logger, method_table: $$$64$moonbitlang$47$core$47$builtin$46$StringBuilder$36$as$36$64$moonbitlang$47$core$47$builtin$46$Logger });
  return logger.val;
}
function moonbitlang$core$builtin$$Show$to_string$11$(self) {
  const logger = moonbitlang$core$builtin$$StringBuilder$new$46$inner(0);
  moonbitlang$core$builtin$$Show$output$12$(self, { self: logger, method_table: $$$64$moonbitlang$47$core$47$builtin$46$StringBuilder$36$as$36$64$moonbitlang$47$core$47$builtin$46$Logger });
  return logger.val;
}
function moonbitlang$core$int$$Int$to_string$46$inner(self, radix) {
  return moonbitlang$core$builtin$$int_to_string_js(self, radix);
}
function moonbitlang$core$builtin$$Show$to_string$13$(self) {
  return self.str.substring(self.start, self.end);
}
function moonbitlang$core$string$$String$from_array(chars) {
  const buf = moonbitlang$core$builtin$$StringBuilder$new$46$inner(Math.imul(chars.end - chars.start | 0, 4) | 0);
  const _len = chars.end - chars.start | 0;
  let _tmp = 0;
  while (true) {
    const _i = _tmp;
    if (_i < _len) {
      const c = chars.buf[chars.start + _i | 0];
      moonbitlang$core$builtin$$Logger$write_char$2$(buf, c);
      _tmp = _i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  return buf.val;
}
function moonbitlang$core$string$$String$char_length_eq$46$inner(self, len, start_offset, end_offset) {
  let end_offset$2;
  if (end_offset === undefined) {
    end_offset$2 = self.length;
  } else {
    const _Some = end_offset;
    end_offset$2 = _Some;
  }
  let _tmp = start_offset;
  let _tmp$2 = 0;
  while (true) {
    const index = _tmp;
    const count = _tmp$2;
    if (index < end_offset$2 && count < len) {
      const c1 = self.charCodeAt(index);
      if (moonbitlang$core$uint16$$UInt16$is_leading_surrogate(c1) && (index + 1 | 0) < end_offset$2) {
        const _tmp$3 = index + 1 | 0;
        const c2 = self.charCodeAt(_tmp$3);
        if (moonbitlang$core$uint16$$UInt16$is_trailing_surrogate(c2)) {
          _tmp = index + 2 | 0;
          _tmp$2 = count + 1 | 0;
          continue;
        } else {
          moonbitlang$core$builtin$$abort$4$("invalid surrogate pair", "@moonbitlang/core/builtin:string.mbt:424:9-424:40");
        }
      }
      _tmp = index + 1 | 0;
      _tmp$2 = count + 1 | 0;
      continue;
    } else {
      return count === len && index === end_offset$2;
    }
  }
}
function moonbitlang$core$builtin$$Logger$write_view$2$(self, str) {
  const _bind = self;
  _bind.val = `${_bind.val}${moonbitlang$core$builtin$$Show$to_string$13$(str)}`;
}
function moonbitlang$core$builtin$$boyer_moore_horspool_find(haystack, needle) {
  const haystack_len = haystack.end - haystack.start | 0;
  const needle_len = needle.end - needle.start | 0;
  if (needle_len > 0) {
    if (haystack_len >= needle_len) {
      const skip_table = $make_array_len_and_init(256, needle_len);
      const _end4200 = needle_len - 1 | 0;
      let _tmp = 0;
      while (true) {
        const i = _tmp;
        if (i < _end4200) {
          const _tmp$2 = needle.str;
          const _tmp$3 = needle.start + i | 0;
          const _tmp$4 = _tmp$2.charCodeAt(_tmp$3) & 255;
          $bound_check(skip_table, _tmp$4);
          skip_table[_tmp$4] = (needle_len - 1 | 0) - i | 0;
          _tmp = i + 1 | 0;
          continue;
        } else {
          break;
        }
      }
      let _tmp$2 = 0;
      while (true) {
        const i = _tmp$2;
        if (i <= (haystack_len - needle_len | 0)) {
          const _end4206 = needle_len - 1 | 0;
          let _tmp$3 = 0;
          while (true) {
            const j = _tmp$3;
            if (j <= _end4206) {
              const _p = i + j | 0;
              const _tmp$4 = haystack.str;
              const _tmp$5 = haystack.start + _p | 0;
              const _p$2 = _tmp$4.charCodeAt(_tmp$5);
              const _tmp$6 = needle.str;
              const _tmp$7 = needle.start + j | 0;
              const _p$3 = _tmp$6.charCodeAt(_tmp$7);
              if (_p$2 !== _p$3) {
                break;
              }
              _tmp$3 = j + 1 | 0;
              continue;
            } else {
              return i;
            }
          }
          const _p = (i + needle_len | 0) - 1 | 0;
          const _tmp$4 = haystack.str;
          const _tmp$5 = haystack.start + _p | 0;
          const _tmp$6 = _tmp$4.charCodeAt(_tmp$5) & 255;
          $bound_check(skip_table, _tmp$6);
          _tmp$2 = i + skip_table[_tmp$6] | 0;
          continue;
        } else {
          break;
        }
      }
      return undefined;
    } else {
      return undefined;
    }
  } else {
    return moonbitlang$core$builtin$$boyer_moore_horspool_find$46$constr$47$198;
  }
}
function moonbitlang$core$builtin$$brute_force_find(haystack, needle) {
  const haystack_len = haystack.end - haystack.start | 0;
  const needle_len = needle.end - needle.start | 0;
  if (needle_len > 0) {
    if (haystack_len >= needle_len) {
      const _p = 0;
      const _tmp = needle.str;
      const _tmp$2 = needle.start + _p | 0;
      const needle_first = _tmp.charCodeAt(_tmp$2);
      const forward_len = haystack_len - needle_len | 0;
      let i = 0;
      while (true) {
        if (i <= forward_len) {
          while (true) {
            let _tmp$3;
            if (i <= forward_len) {
              const _p$2 = i;
              const _tmp$4 = haystack.str;
              const _tmp$5 = haystack.start + _p$2 | 0;
              const _p$3 = _tmp$4.charCodeAt(_tmp$5);
              _tmp$3 = _p$3 !== needle_first;
            } else {
              _tmp$3 = false;
            }
            if (_tmp$3) {
              i = i + 1 | 0;
              continue;
            } else {
              break;
            }
          }
          if (i <= forward_len) {
            let _tmp$3 = 1;
            while (true) {
              const j = _tmp$3;
              if (j < needle_len) {
                const _p$2 = i + j | 0;
                const _tmp$4 = haystack.str;
                const _tmp$5 = haystack.start + _p$2 | 0;
                const _p$3 = _tmp$4.charCodeAt(_tmp$5);
                const _tmp$6 = needle.str;
                const _tmp$7 = needle.start + j | 0;
                const _p$4 = _tmp$6.charCodeAt(_tmp$7);
                if (_p$3 !== _p$4) {
                  break;
                }
                _tmp$3 = j + 1 | 0;
                continue;
              } else {
                return i;
              }
            }
            i = i + 1 | 0;
          }
          continue;
        } else {
          break;
        }
      }
      return undefined;
    } else {
      return undefined;
    }
  } else {
    return moonbitlang$core$builtin$$brute_force_find$46$constr$47$212;
  }
}
function moonbitlang$core$string$$StringView$find(self, str) {
  return (str.end - str.start | 0) <= 4 ? moonbitlang$core$builtin$$brute_force_find(self, str) : moonbitlang$core$builtin$$boyer_moore_horspool_find(self, str);
}
function moonbitlang$core$array$$Array$push$8$(self, value) {
  moonbitlang$core$builtin$$JSArray$push(self, value);
}
function moonbitlang$core$array$$Array$push$14$(self, value) {
  moonbitlang$core$builtin$$JSArray$push(self, value);
}
function moonbitlang$core$array$$Array$push$15$(self, value) {
  moonbitlang$core$builtin$$JSArray$push(self, value);
}
function moonbitlang$core$array$$Array$push$16$(self, value) {
  moonbitlang$core$builtin$$JSArray$push(self, value);
}
function moonbitlang$core$array$$Array$push$5$(self, value) {
  moonbitlang$core$builtin$$JSArray$push(self, value);
}
function moonbitlang$core$builtin$$Iter$next$15$(self) {
  const _func = self;
  return _func();
}
function moonbitlang$core$builtin$$Iter$next$16$(self) {
  const _func = self;
  return _func();
}
function moonbitlang$core$string$$StringView$contains(self, str) {
  const _bind = moonbitlang$core$string$$StringView$find(self, str);
  return !(_bind === undefined);
}
function moonbitlang$core$string$$String$contains(self, str) {
  return moonbitlang$core$string$$StringView$contains({ str: self, start: 0, end: self.length }, str);
}
function moonbitlang$core$string$$String$iter(self) {
  const len = self.length;
  const index = { val: 0 };
  const _p = () => {
    if (index.val < len) {
      const _tmp = index.val;
      const c1 = self.charCodeAt(_tmp);
      if (moonbitlang$core$uint16$$UInt16$is_leading_surrogate(c1) && (index.val + 1 | 0) < len) {
        const _tmp$2 = index.val + 1 | 0;
        const c2 = self.charCodeAt(_tmp$2);
        if (moonbitlang$core$uint16$$UInt16$is_trailing_surrogate(c2)) {
          const c = moonbitlang$core$builtin$$code_point_of_surrogate_pair(c1, c2);
          index.val = index.val + 2 | 0;
          return c;
        }
      }
      index.val = index.val + 1 | 0;
      return c1;
    } else {
      return -1;
    }
  };
  return _p;
}
function moonbitlang$core$builtin$$Show$to_string$16$(self) {
  return String.fromCodePoint(self);
}
function moonbitlang$core$builtin$$ToStringView$to_string_view$8$(self) {
  return { str: self, start: 0, end: self.length };
}
function moonbitlang$core$builtin$$Compare$compare$8$(self, other) {
  const len = self.length;
  const _bind = $compare_int(len, other.length);
  if (_bind === 0) {
    let _tmp = 0;
    while (true) {
      const i = _tmp;
      if (i < len) {
        const _p = self.charCodeAt(i);
        const _p$2 = other.charCodeAt(i);
        const order = $compare_int(_p, _p$2);
        if (order !== 0) {
          return order;
        }
        _tmp = i + 1 | 0;
        continue;
      } else {
        break;
      }
    }
    return 0;
  } else {
    return _bind;
  }
}
function moonbitlang$core$builtin$$Show$output$12$(self, logger) {
  logger.method_table.method_0(logger.self, moonbitlang$core$int$$Int$to_string$46$inner(self, 10));
}
function moonbitlang$core$array$$ArrayView$iter$15$(self) {
  const i = { val: 0 };
  const _p = () => {
    if (i.val < (self.end - self.start | 0)) {
      const elem = self.buf[self.start + i.val | 0];
      i.val = i.val + 1 | 0;
      return elem;
    } else {
      return undefined;
    }
  };
  return _p;
}
function moonbitlang$core$array$$Array$iter$15$(self) {
  return moonbitlang$core$array$$ArrayView$iter$15$({ buf: self, start: 0, end: self.length });
}
function moonbitlang$core$array$$ArrayView$at$5$(self, index) {
  if (index >= 0 && index < (self.end - self.start | 0)) {
    const _tmp = self.buf;
    const _tmp$2 = self.start + index | 0;
    $bound_check(_tmp, _tmp$2);
    return _tmp[_tmp$2];
  } else {
    return moonbitlang$core$builtin$$abort$5$(`index out of bounds: the len is from 0 to ${moonbitlang$core$builtin$$Show$to_string$11$(self.end - self.start | 0)} but the index is ${moonbitlang$core$builtin$$Show$to_string$11$(index)}`, "@moonbitlang/core/builtin:arrayview.mbt:124:5-126:6");
  }
}
function moonbitlang$core$bytes$$Bytes$makei$17$(length, value) {
  if (length <= 0) {
    return $bytes_literal$0;
  }
  const arr = $makebytes(length, value(0));
  let _tmp = 1;
  while (true) {
    const i = _tmp;
    if (i < length) {
      $bound_check(arr, i);
      arr[i] = value(i);
      _tmp = i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  return arr;
}
function moonbitlang$core$builtin$$println$8$(input) {
  console.log(input);
}
function moonbitlang$core$bytes$$Bytes$from_array(arr) {
  return moonbitlang$core$bytes$$Bytes$makei$17$(arr.end - arr.start | 0, (i) => moonbitlang$core$array$$ArrayView$at$5$(arr, i));
}
function moonbitlang$core$builtin$$Show$output$18$(self, logger) {
  const pkg = self.pkg;
  const _data = pkg.str;
  const _start = pkg.start;
  const _end = _start + (pkg.end - pkg.start | 0) | 0;
  let _cursor = _start;
  let accept_state = -1;
  let match_end = -1;
  let match_tag_saver_0 = -1;
  let tag_0 = -1;
  let _bind;
  _L: {
    _L$2: {
      _L$3: while (true) {
        if (_cursor < _end) {
          _L$4: {
            _L$5: {
              const _p = _cursor;
              const next_char = _data.charCodeAt(_p);
              _cursor = _cursor + 1 | 0;
              if (next_char < 55296) {
                if (next_char < 47) {
                  break _L$5;
                } else {
                  if (next_char > 47) {
                    break _L$5;
                  } else {
                    _L$6: while (true) {
                      tag_0 = _cursor;
                      if (_cursor < _end) {
                        _L$7: {
                          const _p$2 = _cursor;
                          const next_char$2 = _data.charCodeAt(_p$2);
                          _cursor = _cursor + 1 | 0;
                          if (next_char$2 < 55296) {
                            if (next_char$2 < 47) {
                              break _L$7;
                            } else {
                              if (next_char$2 > 47) {
                                break _L$7;
                              } else {
                                while (true) {
                                  if (_cursor < _end) {
                                    _L$8: {
                                      const _p$3 = _cursor;
                                      const next_char$3 = _data.charCodeAt(_p$3);
                                      _cursor = _cursor + 1 | 0;
                                      if (next_char$3 < 56319) {
                                        if (next_char$3 < 55296) {
                                          break _L$8;
                                        } else {
                                          if (_cursor < _end) {
                                            const _p$4 = _cursor;
                                            const next_char$4 = _data.charCodeAt(_p$4);
                                            _cursor = _cursor + 1 | 0;
                                            if (next_char$4 < 56320) {
                                              break _L$2;
                                            } else {
                                              if (next_char$4 > 65535) {
                                                break _L$2;
                                              } else {
                                                continue;
                                              }
                                            }
                                          } else {
                                            break _L$2;
                                          }
                                        }
                                      } else {
                                        if (next_char$3 > 56319) {
                                          if (next_char$3 < 65536) {
                                            break _L$8;
                                          } else {
                                            break _L$2;
                                          }
                                        } else {
                                          if (_cursor < _end) {
                                            const _p$4 = _cursor;
                                            const next_char$4 = _data.charCodeAt(_p$4);
                                            _cursor = _cursor + 1 | 0;
                                            if (next_char$4 < 56320) {
                                              break _L$2;
                                            } else {
                                              if (next_char$4 > 57343) {
                                                break _L$2;
                                              } else {
                                                continue;
                                              }
                                            }
                                          } else {
                                            break _L$2;
                                          }
                                        }
                                      }
                                    }
                                    continue;
                                  } else {
                                    match_tag_saver_0 = tag_0;
                                    accept_state = 0;
                                    match_end = _cursor;
                                    break _L$2;
                                  }
                                }
                              }
                            }
                          } else {
                            if (next_char$2 > 56318) {
                              if (next_char$2 < 57344) {
                                if (_cursor < _end) {
                                  const _p$3 = _cursor;
                                  const next_char$3 = _data.charCodeAt(_p$3);
                                  _cursor = _cursor + 1 | 0;
                                  if (next_char$3 < 56320) {
                                    break _L$2;
                                  } else {
                                    if (next_char$3 > 57343) {
                                      break _L$2;
                                    } else {
                                      continue;
                                    }
                                  }
                                } else {
                                  break _L$2;
                                }
                              } else {
                                if (next_char$2 > 65535) {
                                  break _L$2;
                                } else {
                                  break _L$7;
                                }
                              }
                            } else {
                              if (_cursor < _end) {
                                const _p$3 = _cursor;
                                const next_char$3 = _data.charCodeAt(_p$3);
                                _cursor = _cursor + 1 | 0;
                                if (next_char$3 < 56320) {
                                  break _L$2;
                                } else {
                                  if (next_char$3 > 65535) {
                                    break _L$2;
                                  } else {
                                    continue;
                                  }
                                }
                              } else {
                                break _L$2;
                              }
                            }
                          }
                        }
                        continue;
                      } else {
                        break _L$2;
                      }
                    }
                  }
                }
              } else {
                if (next_char > 56318) {
                  if (next_char < 57344) {
                    if (_cursor < _end) {
                      const _p$2 = _cursor;
                      const next_char$2 = _data.charCodeAt(_p$2);
                      _cursor = _cursor + 1 | 0;
                      if (next_char$2 < 56320) {
                        break _L$2;
                      } else {
                        if (next_char$2 > 57343) {
                          break _L$2;
                        } else {
                          continue;
                        }
                      }
                    } else {
                      break _L$2;
                    }
                  } else {
                    if (next_char > 65535) {
                      break _L$2;
                    } else {
                      break _L$5;
                    }
                  }
                } else {
                  if (_cursor < _end) {
                    const _p$2 = _cursor;
                    const next_char$2 = _data.charCodeAt(_p$2);
                    _cursor = _cursor + 1 | 0;
                    if (next_char$2 < 56320) {
                      break _L$2;
                    } else {
                      if (next_char$2 > 65535) {
                        break _L$2;
                      } else {
                        continue;
                      }
                    }
                  } else {
                    break _L$2;
                  }
                }
              }
              break _L$4;
            }
            continue;
          }
        } else {
          break _L$2;
        }
      }
      break _L;
    }
    if (accept_state === 0) {
      let package_name;
      let _try_err;
      _L$3: {
        _L$4: {
          const _bind$2 = moonbitlang$core$string$$String$sub(_data, match_tag_saver_0 + 1 | 0, match_end);
          if (_bind$2.$tag === 1) {
            const _ok = _bind$2;
            package_name = _ok._0;
          } else {
            const _err = _bind$2;
            const _tmp = _err._0;
            _try_err = _tmp;
            break _L$4;
          }
          break _L$3;
        }
        package_name = $panic();
      }
      let module_name;
      let _try_err$2;
      _L$4: {
        _L$5: {
          const _bind$2 = moonbitlang$core$string$$String$sub(_data, _start, match_tag_saver_0);
          if (_bind$2.$tag === 1) {
            const _ok = _bind$2;
            module_name = _ok._0;
          } else {
            const _err = _bind$2;
            const _tmp = _err._0;
            _try_err$2 = _tmp;
            break _L$5;
          }
          break _L$4;
        }
        module_name = $panic();
      }
      _bind = { _0: module_name, _1: package_name };
    } else {
      _bind = { _0: pkg, _1: undefined };
    }
  }
  const _module_name = _bind._0;
  const _package_name = _bind._1;
  if (_package_name === undefined) {
  } else {
    const _Some = _package_name;
    const _pkg_name = _Some;
    logger.method_table.method_2(logger.self, _pkg_name);
    logger.method_table.method_3(logger.self, 47);
  }
  logger.method_table.method_2(logger.self, self.filename);
  logger.method_table.method_3(logger.self, 58);
  logger.method_table.method_2(logger.self, self.start_line);
  logger.method_table.method_3(logger.self, 58);
  logger.method_table.method_2(logger.self, self.start_column);
  logger.method_table.method_3(logger.self, 45);
  logger.method_table.method_2(logger.self, self.end_line);
  logger.method_table.method_3(logger.self, 58);
  logger.method_table.method_2(logger.self, self.end_column);
  logger.method_table.method_3(logger.self, 64);
  logger.method_table.method_2(logger.self, _module_name);
}
function moonbitlang$core$builtin$$Show$output$10$(self, logger) {
  moonbitlang$core$builtin$$Show$output$18$(moonbitlang$core$builtin$$SourceLocRepr$parse(self), logger);
}
function moonbitlang$core$array$$ArrayView$join$8$(self, separator) {
  if ((self.end - self.start | 0) === 0) {
    return "";
  } else {
    const _hd = self.buf[self.start];
    const _bind = self.buf;
    const _bind$2 = 1 + self.start | 0;
    const _bind$3 = self.end;
    const _x = { buf: _bind, start: _bind$2, end: _bind$3 };
    const hd = moonbitlang$core$builtin$$ToStringView$to_string_view$8$(_hd);
    let size_hint = hd.end - hd.start | 0;
    const _len = _x.end - _x.start | 0;
    let _tmp = 0;
    while (true) {
      const _i = _tmp;
      if (_i < _len) {
        const s = _bind[_bind$2 + _i | 0];
        const _tmp$2 = size_hint;
        const _p = moonbitlang$core$builtin$$ToStringView$to_string_view$8$(s);
        size_hint = _tmp$2 + ((_p.end - _p.start | 0) + (separator.end - separator.start | 0) | 0) | 0;
        _tmp = _i + 1 | 0;
        continue;
      } else {
        break;
      }
    }
    size_hint = size_hint << 1;
    const buf = moonbitlang$core$builtin$$StringBuilder$new$46$inner(size_hint);
    moonbitlang$core$builtin$$Logger$write_view$2$(buf, hd);
    if (moonbitlang$core$string$$String$char_length_eq$46$inner(separator.str, 0, separator.start, separator.end)) {
      const _len$2 = _x.end - _x.start | 0;
      let _tmp$2 = 0;
      while (true) {
        const _i = _tmp$2;
        if (_i < _len$2) {
          const s = _bind[_bind$2 + _i | 0];
          const s$2 = moonbitlang$core$builtin$$ToStringView$to_string_view$8$(s);
          moonbitlang$core$builtin$$Logger$write_view$2$(buf, s$2);
          _tmp$2 = _i + 1 | 0;
          continue;
        } else {
          break;
        }
      }
    } else {
      const _len$2 = _x.end - _x.start | 0;
      let _tmp$2 = 0;
      while (true) {
        const _i = _tmp$2;
        if (_i < _len$2) {
          const s = _bind[_bind$2 + _i | 0];
          const s$2 = moonbitlang$core$builtin$$ToStringView$to_string_view$8$(s);
          moonbitlang$core$builtin$$Logger$write_view$2$(buf, separator);
          moonbitlang$core$builtin$$Logger$write_view$2$(buf, s$2);
          _tmp$2 = _i + 1 | 0;
          continue;
        } else {
          break;
        }
      }
    }
    return buf.val;
  }
}
function moonbitlang$core$array$$Array$push_iter$15$(self, iter) {
  while (true) {
    const _bind = moonbitlang$core$builtin$$Iter$next$15$(iter);
    if (_bind === undefined) {
      return;
    } else {
      const _Some = _bind;
      const _x = _Some;
      moonbitlang$core$array$$Array$push$15$(self, _x);
      continue;
    }
  }
}
function moonbitlang$core$array$$Array$join$8$(self, separator) {
  return moonbitlang$core$array$$ArrayView$join$8$({ buf: self, start: 0, end: self.length }, separator);
}
function ZSeanYves$Doclint$src$core$$run_rules(ctx, doc, rules) {
  const all = [];
  const _len = rules.length;
  let _tmp = 0;
  while (true) {
    const _i = _tmp;
    if (_i < _len) {
      const r = rules[_i];
      const issues = r.method_table.method_1(r.self, ctx, doc);
      moonbitlang$core$array$$Array$push_iter$15$(all, moonbitlang$core$array$$Array$iter$15$(issues));
      _tmp = _i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  return all;
}
function ZSeanYves$Doclint$src$core$$severity_to_string(s) {
  switch (s) {
    case 0: {
      return "Info";
    }
    case 1: {
      return "Warn";
    }
    default: {
      return "Error";
    }
  }
}
function ZSeanYves$Doclint$src$core$$escape_str(s) {
  let out = "";
  const _it = moonbitlang$core$string$$String$iter(s);
  while (true) {
    const _bind = moonbitlang$core$builtin$$Iter$next$16$(_it);
    if (_bind === -1) {
      break;
    } else {
      const _Some = _bind;
      const _ch = _Some;
      switch (_ch) {
        case 34: {
          out = `${out}\\\"`;
          break;
        }
        case 92: {
          out = `${out}\\\\`;
          break;
        }
        case 10: {
          out = `${out}\\n`;
          break;
        }
        case 13: {
          out = `${out}\\r`;
          break;
        }
        case 9: {
          out = `${out}\\t`;
          break;
        }
        default: {
          out = `${out}${moonbitlang$core$builtin$$Show$to_string$16$(_ch)}`;
        }
      }
      continue;
    }
  }
  return out;
}
function ZSeanYves$Doclint$src$core$$quote(s) {
  return `\"${ZSeanYves$Doclint$src$core$$escape_str(s)}\"`;
}
function ZSeanYves$Doclint$src$core$$location_to_json(loc) {
  return `{\"page\":${moonbitlang$core$int$$Int$to_string$46$inner(loc.page, 10)},\"span_start\":${moonbitlang$core$int$$Int$to_string$46$inner(loc.span_start, 10)},\"span_end\":${moonbitlang$core$int$$Int$to_string$46$inner(loc.span_end, 10)}}`;
}
function ZSeanYves$Doclint$src$core$$issue_to_json(it) {
  const _bind = it.location;
  let loc_json;
  if (_bind === undefined) {
    loc_json = "null";
  } else {
    const _Some = _bind;
    const _loc = _Some;
    loc_json = ZSeanYves$Doclint$src$core$$location_to_json(_loc);
  }
  return `{\"rule_id\":${ZSeanYves$Doclint$src$core$$quote(it.rule_id)},\"severity\":${ZSeanYves$Doclint$src$core$$quote(ZSeanYves$Doclint$src$core$$severity_to_string(it.severity))},\"message\":${ZSeanYves$Doclint$src$core$$quote(it.message)},\"location\":${loc_json}}`;
}
function ZSeanYves$Doclint$src$core$$issues_to_json(issues) {
  const parts = [];
  const _len = issues.length;
  let _tmp = 0;
  while (true) {
    const _i = _tmp;
    if (_i < _len) {
      const it = issues[_i];
      moonbitlang$core$array$$Array$push$8$(parts, ZSeanYves$Doclint$src$core$$issue_to_json(it));
      _tmp = _i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  return `{\"issues\":[${moonbitlang$core$array$$Array$join$8$(parts, { str: ZSeanYves$Doclint$src$core$$issues_to_json$46$42$bind$124$202, start: 0, end: ZSeanYves$Doclint$src$core$$issues_to_json$46$42$bind$124$202.length })}]}`;
}
function ZSeanYves$Doclint$src$core$$escape_html(s) {
  let out = "";
  const _it = moonbitlang$core$string$$String$iter(s);
  while (true) {
    const _bind = moonbitlang$core$builtin$$Iter$next$16$(_it);
    if (_bind === -1) {
      break;
    } else {
      const _Some = _bind;
      const _ch = _Some;
      switch (_ch) {
        case 60: {
          out = `${out}&lt;`;
          break;
        }
        case 62: {
          out = `${out}&gt;`;
          break;
        }
        case 38: {
          out = `${out}&amp;`;
          break;
        }
        case 34: {
          out = `${out}&quot;`;
          break;
        }
        case 39: {
          out = `${out}&#39;`;
          break;
        }
        default: {
          out = `${out}${moonbitlang$core$builtin$$Show$to_string$16$(_ch)}`;
        }
      }
      continue;
    }
  }
  return out;
}
function ZSeanYves$Doclint$src$core$$severity_badge(sev) {
  switch (sev) {
    case 2: {
      return "<span class=\"badge error\">Error</span>";
    }
    case 1: {
      return "<span class=\"badge warn\">Warn</span>";
    }
    default: {
      return "<span class=\"badge info\">Info</span>";
    }
  }
}
function ZSeanYves$Doclint$src$core$$issue_row_html(it) {
  const badge = ZSeanYves$Doclint$src$core$$severity_badge(it.severity);
  const _bind = it.location;
  let loc;
  if (_bind === undefined) {
    loc = "";
  } else {
    const _Some = _bind;
    const _l = _Some;
    loc = `page ${moonbitlang$core$int$$Int$to_string$46$inner(_l.page, 10)} [${moonbitlang$core$int$$Int$to_string$46$inner(_l.span_start, 10)},${moonbitlang$core$int$$Int$to_string$46$inner(_l.span_end, 10)})`;
  }
  return `<tr><td><code>${ZSeanYves$Doclint$src$core$$escape_html(it.rule_id)}</code></td><td>${badge}</td><td>${ZSeanYves$Doclint$src$core$$escape_html(it.message)}</td><td>${ZSeanYves$Doclint$src$core$$escape_html(loc)}</td></tr>`;
}
function ZSeanYves$Doclint$src$core$$section_table(title, rows) {
  return rows === "" ? `<div class=\"card\"><h2>${ZSeanYves$Doclint$src$core$$escape_html(title)}</h2><div class=\"muted\">(none)</div></div>` : `<div class=\"card\"><h2>${ZSeanYves$Doclint$src$core$$escape_html(title)}</h2><table><thead><tr><th>rule_id</th><th>severity</th><th>message</th><th>location</th></tr></thead><tbody>${rows}</tbody></table></div>`;
}
function ZSeanYves$Doclint$src$core$$issues_to_html(issues) {
  let err = 0;
  let warn = 0;
  let info = 0;
  const _len = issues.length;
  let _tmp = 0;
  while (true) {
    const _i = _tmp;
    if (_i < _len) {
      const it = issues[_i];
      const _bind = it.severity;
      switch (_bind) {
        case 2: {
          err = err + 1 | 0;
          break;
        }
        case 1: {
          warn = warn + 1 | 0;
          break;
        }
        default: {
          info = info + 1 | 0;
        }
      }
      _tmp = _i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  let err_rows = "";
  let warn_rows = "";
  let info_rows = "";
  const _len$2 = issues.length;
  let _tmp$2 = 0;
  while (true) {
    const _i = _tmp$2;
    if (_i < _len$2) {
      const it = issues[_i];
      const row = ZSeanYves$Doclint$src$core$$issue_row_html(it);
      const _bind = it.severity;
      switch (_bind) {
        case 2: {
          err_rows = `${err_rows}${row}`;
          break;
        }
        case 1: {
          warn_rows = `${warn_rows}${row}`;
          break;
        }
        default: {
          info_rows = `${info_rows}${row}`;
        }
      }
      _tmp$2 = _i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  const total = moonbitlang$core$int$$Int$to_string$46$inner(issues.length, 10);
  return `<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Doclint Report</title><style>body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial; margin:0; padding:24px; background:#fafafa;}.card{background:#fff; border:1px solid #eee; border-radius:12px; padding:16px; margin-bottom:16px; box-shadow:0 1px 2px rgba(0,0,0,.04);}.kpi{display:flex; gap:12px; flex-wrap:wrap;}.kpi .box{padding:10px 12px; border:1px solid #eee; border-radius:10px; background:#fff;}h1{font-size:20px; margin:0 0 12px 0;}h2{font-size:16px; margin:16px 0 8px 0;}table{width:100%; border-collapse:collapse; background:#fff; border:1px solid #eee; border-radius:12px; overflow:hidden;}th,td{padding:10px 12px; border-bottom:1px solid #eee; text-align:left; vertical-align:top;}th{background:#f6f7f9; font-weight:600;}tr:last-child td{border-bottom:none;}.badge{display:inline-block; padding:2px 8px; border-radius:999px; font-size:12px; border:1px solid #eee; background:#fff;}.badge.error{border-color:#f3c2c2; background:#fff5f5;}.badge.warn{border-color:#f2d6a5; background:#fffaf2;}.badge.info{border-color:#b8d4ff; background:#f3f8ff;}.muted{color:#666; font-size:12px;}</style></head><body><div class=\"card\"><h1>Doclint Report</h1><div class=\"kpi\"><div class=\"box\"><div class=\"muted\">Total</div><div>${total}</div></div><div class=\"box\"><div class=\"muted\">Error</div><div>${moonbitlang$core$int$$Int$to_string$46$inner(err, 10)}</div></div><div class=\"box\"><div class=\"muted\">Warn</div><div>${moonbitlang$core$int$$Int$to_string$46$inner(warn, 10)}</div></div><div class=\"box\"><div class=\"muted\">Info</div><div>${moonbitlang$core$int$$Int$to_string$46$inner(info, 10)}</div></div></div></div>${ZSeanYves$Doclint$src$core$$section_table("Errors", err_rows)}${ZSeanYves$Doclint$src$core$$section_table("Warnings", warn_rows)}${ZSeanYves$Doclint$src$core$$section_table("Infos", info_rows)}</body></html>`;
}
function ZSeanYves$Doclint$src$core$$slice(s, start, end_) {
  let out = "";
  let i = 0;
  const _it = moonbitlang$core$string$$String$iter(s);
  while (true) {
    const _bind = moonbitlang$core$builtin$$Iter$next$16$(_it);
    if (_bind === -1) {
      break;
    } else {
      const _Some = _bind;
      const _ch = _Some;
      if (i >= start && i < end_) {
        out = `${out}${moonbitlang$core$builtin$$Show$to_string$16$(_ch)}`;
      }
      i = i + 1 | 0;
      continue;
    }
  }
  return out;
}
function ZSeanYves$Doclint$src$core$$find_char(s, ch, start) {
  let i = start;
  while (true) {
    if (i < s.length) {
      if (ZSeanYves$Doclint$src$core$$slice(s, i, i + 1 | 0) === moonbitlang$core$builtin$$Show$to_string$16$(ch)) {
        return i;
      }
      i = i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  return undefined;
}
function ZSeanYves$Doclint$src$core$$find_sub(s, pat, start) {
  const n = s.length;
  const m = pat.length;
  if (m === 0) {
    return start;
  }
  let i = start;
  while (true) {
    if ((i + m | 0) <= n) {
      if (ZSeanYves$Doclint$src$core$$slice(s, i, i + m | 0) === pat) {
        return i;
      }
      i = i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  return undefined;
}
function ZSeanYves$Doclint$src$core$$read_json_string(s, start) {
  let out = "";
  let i = start;
  while (true) {
    if (i < s.length) {
      const ch = ZSeanYves$Doclint$src$core$$slice(s, i, i + 1 | 0);
      if (ch === "\"") {
        return { _0: out, _1: i + 1 | 0 };
      } else {
        if (ch === "\\") {
          if ((i + 1 | 0) >= s.length) {
            return undefined;
          }
          const esc = ZSeanYves$Doclint$src$core$$slice(s, i + 1 | 0, i + 2 | 0);
          switch (esc) {
            case "n": {
              out = `${out}\n`;
              break;
            }
            case "r": {
              out = `${out}\r`;
              break;
            }
            case "t": {
              out = `${out}\t`;
              break;
            }
            case "\"": {
              out = `${out}\"`;
              break;
            }
            case "\\": {
              out = `${out}\\`;
              break;
            }
            default: {
              out = `${out}${esc}`;
            }
          }
          i = i + 2 | 0;
        } else {
          out = `${out}${ch}`;
          i = i + 1 | 0;
        }
      }
      continue;
    } else {
      break;
    }
  }
  return undefined;
}
function ZSeanYves$Doclint$src$core$$find_string_field(s, key) {
  const pat = `\"${key}\"`;
  const _bind = ZSeanYves$Doclint$src$core$$find_sub(s, pat, 0);
  let kpos;
  if (_bind === undefined) {
    return undefined;
  } else {
    const _Some = _bind;
    kpos = _Some;
  }
  const _bind$2 = ZSeanYves$Doclint$src$core$$find_char(s, 58, kpos + pat.length | 0);
  let colon;
  if (_bind$2 === undefined) {
    return undefined;
  } else {
    const _Some = _bind$2;
    colon = _Some;
  }
  const _bind$3 = ZSeanYves$Doclint$src$core$$find_char(s, 34, colon + 1 | 0);
  let q1;
  if (_bind$3 === undefined) {
    return undefined;
  } else {
    const _Some = _bind$3;
    q1 = _Some;
  }
  const _bind$4 = ZSeanYves$Doclint$src$core$$read_json_string(s, q1 + 1 | 0);
  let _bind$5;
  if (_bind$4 === undefined) {
    return undefined;
  } else {
    const _Some = _bind$4;
    _bind$5 = _Some;
  }
  const _val = _bind$5._0;
  return _val;
}
function ZSeanYves$Doclint$src$core$$parse_int_dec(s) {
  let acc = 0;
  let any = false;
  const _it = moonbitlang$core$string$$String$iter(s);
  while (true) {
    const _bind = moonbitlang$core$builtin$$Iter$next$16$(_it);
    if (_bind === -1) {
      break;
    } else {
      const _Some = _bind;
      const _ch = _Some;
      if (_ch < 48 || _ch > 57) {
        return undefined;
      }
      acc = (Math.imul(acc, 10) | 0) + (_ch - 48 | 0) | 0;
      any = true;
      continue;
    }
  }
  return any ? acc : undefined;
}
function ZSeanYves$Doclint$src$core$$skip_ws(s, i) {
  let n = i;
  while (true) {
    if (n < s.length) {
      const c = ZSeanYves$Doclint$src$core$$slice(s, n, n + 1 | 0);
      if (c === " " || (c === "\n" || (c === "\r" || c === "\t"))) {
        n = n + 1 | 0;
      } else {
        break;
      }
      continue;
    } else {
      break;
    }
  }
  return n;
}
function ZSeanYves$Doclint$src$core$$find_int_field_from(s, key, start) {
  const pat = `\"${key}\"`;
  const _bind = ZSeanYves$Doclint$src$core$$find_sub(s, pat, start);
  let kpos;
  if (_bind === undefined) {
    return undefined;
  } else {
    const _Some = _bind;
    kpos = _Some;
  }
  const _bind$2 = ZSeanYves$Doclint$src$core$$find_char(s, 58, kpos + pat.length | 0);
  let colon;
  if (_bind$2 === undefined) {
    return undefined;
  } else {
    const _Some = _bind$2;
    colon = _Some;
  }
  let i = ZSeanYves$Doclint$src$core$$skip_ws(s, colon + 1 | 0);
  let neg = false;
  if (i < s.length && ZSeanYves$Doclint$src$core$$slice(s, i, i + 1 | 0) === "-") {
    neg = true;
    i = i + 1 | 0;
  }
  let j = i;
  const n = s.length;
  while (true) {
    if (j < n) {
      const c = ZSeanYves$Doclint$src$core$$slice(s, j, j + 1 | 0);
      if (moonbitlang$core$builtin$$Compare$op_ge$9$(c, "0") && moonbitlang$core$builtin$$Compare$op_le$9$(c, "9")) {
        j = j + 1 | 0;
      } else {
        break;
      }
      continue;
    } else {
      break;
    }
  }
  if (j === i) {
    return undefined;
  }
  const digits = ZSeanYves$Doclint$src$core$$slice(s, i, j);
  const _bind$3 = ZSeanYves$Doclint$src$core$$parse_int_dec(digits);
  let num;
  if (_bind$3 === undefined) {
    return undefined;
  } else {
    const _Some = _bind$3;
    num = _Some;
  }
  return neg ? -num | 0 : num;
}
function ZSeanYves$Doclint$src$core$$find_string_field_from(s, key, start) {
  const pat = `\"${key}\"`;
  const _bind = ZSeanYves$Doclint$src$core$$find_sub(s, pat, start);
  let kpos;
  if (_bind === undefined) {
    return undefined;
  } else {
    const _Some = _bind;
    kpos = _Some;
  }
  const _bind$2 = ZSeanYves$Doclint$src$core$$find_char(s, 58, kpos + pat.length | 0);
  let colon;
  if (_bind$2 === undefined) {
    return undefined;
  } else {
    const _Some = _bind$2;
    colon = _Some;
  }
  const _bind$3 = ZSeanYves$Doclint$src$core$$find_char(s, 34, colon + 1 | 0);
  let q1;
  if (_bind$3 === undefined) {
    return undefined;
  } else {
    const _Some = _bind$3;
    q1 = _Some;
  }
  const _bind$4 = ZSeanYves$Doclint$src$core$$read_json_string(s, q1 + 1 | 0);
  let _bind$5;
  if (_bind$4 === undefined) {
    return undefined;
  } else {
    const _Some = _bind$4;
    _bind$5 = _Some;
  }
  const _val = _bind$5._0;
  return _val;
}
function ZSeanYves$Doclint$src$core$$skip_ws_comma(s, i) {
  let n = i;
  while (true) {
    if (n < s.length) {
      const c = ZSeanYves$Doclint$src$core$$slice(s, n, n + 1 | 0);
      if (c === " " || (c === "\n" || (c === "\r" || (c === "\t" || c === ",")))) {
        n = n + 1 | 0;
      } else {
        break;
      }
      continue;
    } else {
      break;
    }
  }
  return n;
}
function ZSeanYves$Doclint$src$core$$parse_pages(s) {
  const _bind = ZSeanYves$Doclint$src$core$$find_sub(s, ZSeanYves$Doclint$src$core$$parse_pages$46$pat$124$82, 0);
  let ppos;
  if (_bind === undefined) {
    return Option$None$2$;
  } else {
    const _Some = _bind;
    ppos = _Some;
  }
  const _bind$2 = ZSeanYves$Doclint$src$core$$find_char(s, 58, ppos + ZSeanYves$Doclint$src$core$$parse_pages$46$pat$124$82.length | 0);
  let colon;
  if (_bind$2 === undefined) {
    return Option$None$2$;
  } else {
    const _Some = _bind$2;
    colon = _Some;
  }
  const _bind$3 = ZSeanYves$Doclint$src$core$$find_char(s, 91, colon + 1 | 0);
  let lb;
  if (_bind$3 === undefined) {
    return Option$None$2$;
  } else {
    const _Some = _bind$3;
    lb = _Some;
  }
  let i = lb + 1 | 0;
  const pages = [];
  while (true) {
    i = ZSeanYves$Doclint$src$core$$skip_ws_comma(s, i);
    if (i >= s.length) {
      return Option$None$2$;
    }
    if (ZSeanYves$Doclint$src$core$$slice(s, i, i + 1 | 0) === "]") {
      break;
    }
    const _bind$4 = ZSeanYves$Doclint$src$core$$find_char(s, 123, i);
    let ob;
    if (_bind$4 === undefined) {
      return Option$None$2$;
    } else {
      const _Some = _bind$4;
      ob = _Some;
    }
    const _bind$5 = ZSeanYves$Doclint$src$core$$find_int_field_from(s, "index", ob);
    let idx;
    if (_bind$5 === undefined) {
      return Option$None$2$;
    } else {
      const _Some = _bind$5;
      idx = _Some;
    }
    const _bind$6 = ZSeanYves$Doclint$src$core$$find_string_field_from(s, "text", ob);
    let txt;
    if (_bind$6 === undefined) {
      return Option$None$2$;
    } else {
      const _Some = _bind$6;
      txt = _Some;
    }
    moonbitlang$core$array$$Array$push$14$(pages, { index: idx, text: txt });
    const _bind$7 = ZSeanYves$Doclint$src$core$$find_char(s, 125, ob + 1 | 0);
    let cb;
    if (_bind$7 === undefined) {
      return Option$None$2$;
    } else {
      const _Some = _bind$7;
      cb = _Some;
    }
    i = cb + 1 | 0;
    continue;
  }
  return new Option$Some$2$(pages);
}
function ZSeanYves$Doclint$src$core$$parse_meta_and_pages(s) {
  const _bind = ZSeanYves$Doclint$src$core$$find_string_field(s, "title");
  let title;
  if (_bind === undefined) {
    title = "";
  } else {
    const _Some = _bind;
    title = _Some;
  }
  const _bind$2 = ZSeanYves$Doclint$src$core$$find_string_field(s, "author");
  let author;
  if (_bind$2 === undefined) {
    author = "";
  } else {
    const _Some = _bind$2;
    author = _Some;
  }
  const _bind$3 = ZSeanYves$Doclint$src$core$$find_string_field(s, "created_at");
  let created_at;
  if (_bind$3 === undefined) {
    created_at = "";
  } else {
    const _Some = _bind$3;
    created_at = _Some;
  }
  const _bind$4 = ZSeanYves$Doclint$src$core$$parse_pages(s);
  let pages;
  if (_bind$4.$tag === 0) {
    pages = [];
  } else {
    const _Some = _bind$4;
    pages = _Some._0;
  }
  return { meta: { title: title, author: author, created_at: created_at }, pages: pages };
}
function ZSeanYves$Doclint$src$core$$parse_doc_json(s) {
  const _bind = ZSeanYves$Doclint$src$core$$parse_meta_and_pages(s);
  if (_bind === undefined) {
    return ZSeanYves$Doclint$src$core$$parse_doc_json$46$constr$47$562;
  } else {
    const _Some = _bind;
    const _doc = _Some;
    return new Result$Ok$0$(_doc);
  }
}
function ZSeanYves$Doclint$src$core$$mk_issue(rule_id, severity, message, location) {
  return { rule_id: rule_id, severity: severity, message: message, location: location };
}
function ZSeanYves$Doclint$src$core$$check_doc(ctx, doc, rules) {
  return ZSeanYves$Doclint$src$core$$run_rules(ctx, doc, rules);
}
function ZSeanYves$Doclint$src$core$$Rule$id$0$(_self) {
  return "thesis.page_limit";
}
function ZSeanYves$Doclint$src$core$$Rule$check$0$(self, _ctx, doc) {
  const n = doc.pages.length;
  return n < self.min || n > self.max ? [ZSeanYves$Doclint$src$core$$mk_issue("thesis.page_limit", 2, "页数不符合要求", undefined)] : [];
}
function ZSeanYves$Doclint$src$core$$Rule$id$1$(self) {
  return self.rid;
}
function ZSeanYves$Doclint$src$core$$Rule$check$1$(self, _ctx, doc) {
  let all_text = "";
  const _arr = doc.pages;
  const _len = _arr.length;
  let _tmp = 0;
  while (true) {
    const _i = _tmp;
    if (_i < _len) {
      const p = _arr[_i];
      all_text = `${all_text}\n${p.text}`;
      _tmp = _i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  const _tmp$2 = all_text;
  const _bind = self.keyword;
  if (moonbitlang$core$string$$String$contains(_tmp$2, { str: _bind, start: 0, end: _bind.length })) {
    return [];
  } else {
    return [ZSeanYves$Doclint$src$core$$mk_issue(self.rid, self.sev, `缺少必需内容：${self.keyword}`, undefined)];
  }
}
function ZSeanYves$Doclint$src$doclint_rules_thesis$$as_rules_ref(rs) {
  return [{ self: rs.page_limit, method_table: $$$64$ZSeanYves$47$Doclint$47$src$47$doclint_rules_thesis$46$PageLimitRule$36$as$36$64$ZSeanYves$47$Doclint$47$src$47$core$46$Rule }, { self: rs.require_abs, method_table: $$$64$ZSeanYves$47$Doclint$47$src$47$doclint_rules_thesis$46$RequireKeywordRule$36$as$36$64$ZSeanYves$47$Doclint$47$src$47$core$46$Rule }];
}
function ZSeanYves$Doclint$src$doclint_rules_contract$$as_rules_ref(rs) {
  return [{ self: rs.page_limit, method_table: $$$64$ZSeanYves$47$Doclint$47$src$47$doclint_rules_thesis$46$PageLimitRule$36$as$36$64$ZSeanYves$47$Doclint$47$src$47$core$46$Rule }, { self: rs.require_abs, method_table: $$$64$ZSeanYves$47$Doclint$47$src$47$doclint_rules_thesis$46$RequireKeywordRule$36$as$36$64$ZSeanYves$47$Doclint$47$src$47$core$46$Rule }];
}
function moonbitlang$x$internal$ffi$$mbt_string_to_utf8_bytes(str, is_filename) {
  const res = [];
  const len = str.length;
  let i = 0;
  while (true) {
    if (i < len) {
      const _tmp = i;
      $bound_check(str, _tmp);
      let c = str.charCodeAt(_tmp);
      if (55296 <= c && c <= 56319) {
        c = c - 55296 | 0;
        i = i + 1 | 0;
        const _tmp$2 = i;
        $bound_check(str, _tmp$2);
        const l = str.charCodeAt(_tmp$2) - 56320 | 0;
        c = ((c << 10) + l | 0) + 65536 | 0;
      }
      if (c < 128) {
        moonbitlang$core$array$$Array$push$5$(res, c & 255);
      } else {
        if (c < 2048) {
          moonbitlang$core$array$$Array$push$5$(res, (192 + (c >> 6) | 0) & 255);
          moonbitlang$core$array$$Array$push$5$(res, (128 + (c & 63) | 0) & 255);
        } else {
          if (c < 65536) {
            moonbitlang$core$array$$Array$push$5$(res, (224 + (c >> 12) | 0) & 255);
            moonbitlang$core$array$$Array$push$5$(res, (128 + (c >> 6 & 63) | 0) & 255);
            moonbitlang$core$array$$Array$push$5$(res, (128 + (c & 63) | 0) & 255);
          } else {
            moonbitlang$core$array$$Array$push$5$(res, (240 + (c >> 18) | 0) & 255);
            moonbitlang$core$array$$Array$push$5$(res, (128 + (c >> 12 & 63) | 0) & 255);
            moonbitlang$core$array$$Array$push$5$(res, (128 + (c >> 6 & 63) | 0) & 255);
            moonbitlang$core$array$$Array$push$5$(res, (128 + (c & 63) | 0) & 255);
          }
        }
      }
      i = i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  if (is_filename) {
    moonbitlang$core$array$$Array$push$5$(res, 0 & 255);
  }
  return moonbitlang$core$bytes$$Bytes$from_array({ buf: res, start: 0, end: res.length });
}
function moonbitlang$x$internal$ffi$$utf8_bytes_to_mbt_string(bytes) {
  const res = [];
  const len = bytes.length;
  let i = 0;
  while (true) {
    if (i < len) {
      const _tmp = i;
      $bound_check(bytes, _tmp);
      let c = bytes[_tmp];
      if (c < 128) {
        moonbitlang$core$array$$Array$push$16$(res, c);
        i = i + 1 | 0;
      } else {
        if (c < 224) {
          if ((i + 1 | 0) >= len) {
            break;
          }
          const _tmp$2 = (c & 31) << 6;
          const _tmp$3 = i + 1 | 0;
          $bound_check(bytes, _tmp$3);
          c = _tmp$2 | bytes[_tmp$3] & 63;
          moonbitlang$core$array$$Array$push$16$(res, c);
          i = i + 2 | 0;
        } else {
          if (c < 240) {
            if ((i + 2 | 0) >= len) {
              break;
            }
            const _tmp$2 = (c & 15) << 12;
            const _tmp$3 = i + 1 | 0;
            $bound_check(bytes, _tmp$3);
            const _tmp$4 = _tmp$2 | (bytes[_tmp$3] & 63) << 6;
            const _tmp$5 = i + 2 | 0;
            $bound_check(bytes, _tmp$5);
            c = _tmp$4 | bytes[_tmp$5] & 63;
            moonbitlang$core$array$$Array$push$16$(res, c);
            i = i + 3 | 0;
          } else {
            if ((i + 3 | 0) >= len) {
              break;
            }
            const _tmp$2 = (c & 7) << 18;
            const _tmp$3 = i + 1 | 0;
            $bound_check(bytes, _tmp$3);
            const _tmp$4 = _tmp$2 | (bytes[_tmp$3] & 63) << 12;
            const _tmp$5 = i + 2 | 0;
            $bound_check(bytes, _tmp$5);
            const _tmp$6 = _tmp$4 | (bytes[_tmp$5] & 63) << 6;
            const _tmp$7 = i + 3 | 0;
            $bound_check(bytes, _tmp$7);
            c = _tmp$6 | bytes[_tmp$7] & 63;
            c = c - 65536 | 0;
            moonbitlang$core$array$$Array$push$16$(res, (c >> 10) + 55296 | 0);
            moonbitlang$core$array$$Array$push$16$(res, (c & 1023) + 56320 | 0);
            i = i + 4 | 0;
          }
        }
      }
      continue;
    } else {
      break;
    }
  }
  return moonbitlang$core$string$$String$from_array({ buf: res, start: 0, end: res.length });
}
function moonbitlang$x$fs$$read_file_to_bytes_internal(path) {
  const res = moonbitlang$x$fs$$read_file_ffi(path);
  if (res === -1) {
    return new Result$Err$3$(new Error$moonbitlang$47$x$47$fs$46$IOError$46$IOError(moonbitlang$x$fs$$get_error_message_ffi()));
  }
  return new Result$Ok$3$(moonbitlang$x$fs$$get_file_content_ffi());
}
function moonbitlang$x$fs$$read_file_to_string_internal$46$inner(path, encoding) {
  if (encoding === "utf8") {
    const _bind = moonbitlang$x$fs$$read_file_to_bytes_internal(path);
    let bytes;
    if (_bind.$tag === 1) {
      const _ok = _bind;
      bytes = _ok._0;
    } else {
      return _bind;
    }
    return new Result$Ok$4$(moonbitlang$x$internal$ffi$$utf8_bytes_to_mbt_string(bytes));
  } else {
    return new Result$Err$4$(new Error$moonbitlang$47$x$47$fs$46$IOError$46$IOError(`Unsupported encoding: ${encoding}, only utf8 is supported for now`));
  }
}
function moonbitlang$x$fs$$write_bytes_to_file_internal(path, content) {
  const res = moonbitlang$x$fs$$write_file_ffi(path, content);
  if (res === -1) {
    return new Result$Err$5$(new Error$moonbitlang$47$x$47$fs$46$IOError$46$IOError(moonbitlang$x$fs$$get_error_message_ffi()));
  } else {
    return new Result$Ok$5$(undefined);
  }
}
function moonbitlang$x$fs$$write_string_to_file_internal$46$inner(path, content, encoding) {
  if (encoding === "utf8") {
    const bytes = moonbitlang$x$internal$ffi$$mbt_string_to_utf8_bytes(content, false);
    return moonbitlang$x$fs$$write_bytes_to_file_internal(path, bytes);
  } else {
    return new Result$Err$5$(new Error$moonbitlang$47$x$47$fs$46$IOError$46$IOError(`Unsupported encoding: ${encoding}, only utf8 is supported for now`));
  }
}
function moonbitlang$x$fs$$path_exists_internal(path) {
  return moonbitlang$x$fs$$path_exists_ffi(path);
}
function moonbitlang$x$fs$$create_dir_internal(path) {
  const res = moonbitlang$x$fs$$create_dir_ffi(path);
  if (res === -1) {
    return new Result$Err$5$(new Error$moonbitlang$47$x$47$fs$46$IOError$46$IOError(moonbitlang$x$fs$$get_error_message_ffi()));
  } else {
    return new Result$Ok$5$(undefined);
  }
}
function moonbitlang$x$fs$$is_dir_internal(path) {
  const res = moonbitlang$x$fs$$is_dir_ffi(path);
  if (res === -1) {
    return new Result$Err$6$(new Error$moonbitlang$47$x$47$fs$46$IOError$46$IOError(moonbitlang$x$fs$$get_error_message_ffi()));
  }
  return new Result$Ok$6$(res === 1);
}
function moonbitlang$x$fs$$read_file_to_string$46$inner(path, encoding) {
  return moonbitlang$x$fs$$read_file_to_string_internal$46$inner(path, encoding);
}
function moonbitlang$x$fs$$write_string_to_file$46$inner(path, content, encoding) {
  return moonbitlang$x$fs$$write_string_to_file_internal$46$inner(path, content, encoding);
}
function moonbitlang$x$fs$$path_exists(path) {
  return moonbitlang$x$fs$$path_exists_internal(path);
}
function moonbitlang$x$fs$$create_dir(path) {
  return moonbitlang$x$fs$$create_dir_internal(path);
}
function moonbitlang$x$fs$$is_dir(path) {
  return moonbitlang$x$fs$$is_dir_internal(path);
}
function moonbitlang$x$sys$internal$ffi$$get_cli_args() {
  return moonbitlang$x$sys$internal$ffi$$get_cli_args_internal();
}
function moonbitlang$x$sys$$get_cli_args() {
  return moonbitlang$x$sys$internal$ffi$$get_cli_args();
}
function moonbitlang$x$sys$$exit(code) {
  moonbitlang$x$sys$internal$ffi$$exit(code);
}
function ZSeanYves$Doclint$src$$read_utf8(path) {
  let s;
  let _try_err;
  _L: {
    _L$2: {
      const _bind = moonbitlang$x$fs$$read_file_to_string$46$inner(path, "utf8");
      if (_bind.$tag === 1) {
        const _ok = _bind;
        s = _ok._0;
      } else {
        const _err = _bind;
        const _tmp = _err._0;
        _try_err = _tmp;
        break _L$2;
      }
      break _L;
    }
    return new Result$Err$7$(`read failed: ${path}`);
  }
  return new Result$Ok$7$(s);
}
function ZSeanYves$Doclint$src$$write_utf8(path, content) {
  let _try_err;
  _L: {
    _L$2: {
      const _bind = moonbitlang$x$fs$$write_string_to_file$46$inner(path, content, "utf8");
      if (_bind.$tag === 1) {
        const _ok = _bind;
        _ok._0;
      } else {
        const _err = _bind;
        const _tmp = _err._0;
        _try_err = _tmp;
        break _L$2;
      }
      break _L;
    }
    return new Result$Err$8$(`write failed: ${path}`);
  }
  return new Result$Ok$8$(undefined);
}
function ZSeanYves$Doclint$src$$ensure_dir(path) {
  if (moonbitlang$x$fs$$path_exists(path)) {
    let _tmp;
    let _try_err;
    _L: {
      _L$2: {
        const _bind = moonbitlang$x$fs$$is_dir(path);
        if (_bind.$tag === 1) {
          const _ok = _bind;
          _tmp = _ok._0;
        } else {
          const _err = _bind;
          const _tmp$2 = _err._0;
          _try_err = _tmp$2;
          break _L$2;
        }
        break _L;
      }
      _tmp = false;
    }
    if (!_tmp) {
      moonbitlang$core$builtin$$println$8$(`path exists but not a dir: ${path}`);
      return;
    } else {
      return;
    }
  } else {
    let _try_err;
    _L: {
      const _bind = moonbitlang$x$fs$$create_dir(path);
      if (_bind.$tag === 1) {
        const _ok = _bind;
        _ok._0;
        return;
      } else {
        const _err = _bind;
        const _tmp = _err._0;
        _try_err = _tmp;
        break _L;
      }
    }
    moonbitlang$core$builtin$$println$8$(`failed to create dir: ${path}`);
    return;
  }
}
function ZSeanYves$Doclint$src$$drop1(xs) {
  const out = [];
  let i = 0;
  const _len = xs.length;
  let _tmp = 0;
  while (true) {
    const _i = _tmp;
    if (_i < _len) {
      const x = xs[_i];
      if (i > 0) {
        moonbitlang$core$array$$Array$push$8$(out, x);
      }
      i = i + 1 | 0;
      _tmp = _i + 1 | 0;
      continue;
    } else {
      break;
    }
  }
  return out;
}
function ZSeanYves$Doclint$src$$print_help() {
  moonbitlang$core$builtin$$println$8$(`Usage:\n  doclint --rules thesis|contract --in <file> --outdir <dir> [--no-html]\n\nExamples:\n  doclint --rules thesis --in test_golden/case1.in.json\n  doclint --rules contract --in test_golden/case2.in.json --outdir out\n\nDefaults:\n  --rules thesis\n  --in test_golden/case2.in.json\n  --outdir out\n`);
}
function ZSeanYves$Doclint$src$$write_out(out_json_path, out_html_path, issues, emit_html) {
  const _bind = ZSeanYves$Doclint$src$$write_utf8(out_json_path, ZSeanYves$Doclint$src$core$$issues_to_json(issues));
  if (_bind.$tag === 1) {
    moonbitlang$core$builtin$$println$8$(`written: ${out_json_path}`);
  } else {
    const _Err = _bind;
    const _msg = _Err._0;
    moonbitlang$core$builtin$$println$8$(_msg);
  }
  if (emit_html) {
    const _bind$2 = ZSeanYves$Doclint$src$$write_utf8(out_html_path, ZSeanYves$Doclint$src$core$$issues_to_html(issues));
    if (_bind$2.$tag === 1) {
      moonbitlang$core$builtin$$println$8$(`written: ${out_html_path}`);
      return;
    } else {
      const _Err = _bind$2;
      const _msg = _Err._0;
      moonbitlang$core$builtin$$println$8$(_msg);
      return;
    }
  } else {
    return;
  }
}
(() => {
  const args = ZSeanYves$Doclint$src$$drop1(moonbitlang$x$sys$$get_cli_args());
  let rules_name = "thesis";
  let in_path = "test_golden/case2.in.json";
  let out_dir = "out";
  let emit_html = true;
  let i = 0;
  while (true) {
    if (i < args.length) {
      const a = moonbitlang$core$array$$Array$at$8$(args, i);
      if (a === "--help" || a === "-h") {
        ZSeanYves$Doclint$src$$print_help();
        moonbitlang$x$sys$$exit(0);
      } else {
        if (a === "--rules") {
          if ((i + 1 | 0) >= args.length) {
            moonbitlang$core$builtin$$println$8$("missing value for --rules");
            ZSeanYves$Doclint$src$$print_help();
            moonbitlang$x$sys$$exit(1);
          }
          rules_name = moonbitlang$core$array$$Array$at$8$(args, i + 1 | 0);
          i = i + 2 | 0;
          continue;
        } else {
          if (a === "--in") {
            if ((i + 1 | 0) >= args.length) {
              moonbitlang$core$builtin$$println$8$("missing value for --in");
              ZSeanYves$Doclint$src$$print_help();
              moonbitlang$x$sys$$exit(1);
            }
            in_path = moonbitlang$core$array$$Array$at$8$(args, i + 1 | 0);
            i = i + 2 | 0;
            continue;
          } else {
            if (a === "--outdir") {
              if ((i + 1 | 0) >= args.length) {
                moonbitlang$core$builtin$$println$8$("missing value for --outdir");
                ZSeanYves$Doclint$src$$print_help();
                moonbitlang$x$sys$$exit(1);
              }
              out_dir = moonbitlang$core$array$$Array$at$8$(args, i + 1 | 0);
              i = i + 2 | 0;
              continue;
            } else {
              if (a === "--no-html") {
                emit_html = false;
                i = i + 1 | 0;
                continue;
              } else {
                moonbitlang$core$builtin$$println$8$(`unknown arg: ${a}`);
                ZSeanYves$Doclint$src$$print_help();
                moonbitlang$x$sys$$exit(1);
              }
            }
          }
        }
      }
      continue;
    } else {
      break;
    }
  }
  ZSeanYves$Doclint$src$$ensure_dir(out_dir);
  const _p = out_dir;
  const _p$2 = "report.json";
  const out_json_path = `${_p}/${_p$2}`;
  const _p$3 = out_dir;
  const _p$4 = "report.html";
  const out_html_path = `${_p$3}/${_p$4}`;
  const doc_json_res = ZSeanYves$Doclint$src$$read_utf8(in_path);
  let doc_json;
  if (doc_json_res.$tag === 0) {
    const _Err = doc_json_res;
    const _msg = _Err._0;
    moonbitlang$core$builtin$$println$8$(_msg);
    moonbitlang$x$sys$$exit(1);
    doc_json = "";
  } else {
    const _Ok = doc_json_res;
    doc_json = _Ok._0;
  }
  const doc_res = ZSeanYves$Doclint$src$core$$parse_doc_json(doc_json);
  let doc;
  if (doc_res.$tag === 0) {
    const _Err = doc_res;
    const _e = _Err._0;
    const issues = [ZSeanYves$Doclint$src$core$$mk_issue("core.parse", 2, `文档 JSON 解析失败: ${_e}`, undefined)];
    ZSeanYves$Doclint$src$$write_out(out_json_path, out_html_path, issues, emit_html);
    moonbitlang$x$sys$$exit(1);
    return;
  } else {
    const _Ok = doc_res;
    doc = _Ok._0;
  }
  const ctx = {};
  let issues;
  if (rules_name === "contract") {
    const rs = ZSeanYves$Doclint$src$doclint_rules_contract$$build_ruleset$46$record$47$597;
    const rules = ZSeanYves$Doclint$src$doclint_rules_contract$$as_rules_ref(rs);
    issues = ZSeanYves$Doclint$src$core$$check_doc(ctx, doc, rules);
  } else {
    const rs = ZSeanYves$Doclint$src$doclint_rules_thesis$$build_ruleset$46$record$47$593;
    const rules = ZSeanYves$Doclint$src$doclint_rules_thesis$$as_rules_ref(rs);
    issues = ZSeanYves$Doclint$src$core$$check_doc(ctx, doc, rules);
  }
  ZSeanYves$Doclint$src$$write_out(out_json_path, out_html_path, issues, emit_html);
  moonbitlang$x$sys$$exit(0);
})();
