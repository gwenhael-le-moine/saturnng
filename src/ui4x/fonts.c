#include "inner.h"

letter_t small_font[ 128 ] = {
    {0,                  0,                   0                },
    {nl_48gx_width,      nl_48gx_height,      nl_48gx_bits     }, /* \x01 == \n gx */
    {comma_48gx_width,   comma_48gx_height,   comma_48gx_bits  }, /* \x02 == comma gx */
    {arrow_48gx_width,   arrow_48gx_height,   arrow_48gx_bits  }, /* \x03 == \-> gx */
    {equal_48gx_width,   equal_48gx_height,   equal_48gx_bits  }, /* \x04 == equal gx */
    {pi_48gx_width,      pi_48gx_height,      pi_48gx_bits     }, /* \x05 == pi gx */
    {arrow_width,        arrow_height,        arrow_bits       }, /* \x06 == left arrow   */
    {diff_width,         diff_height,         diff_bits        }, /* \x07 == differential */
    {integral_width,     integral_height,     integral_bits    }, /* \x08 == integral */
    {sigma_width,        sigma_height,        sigma_bits       }, /* \x09 == sigma */
    {sqr_width,          sqr_height,          sqr_bits         }, /* \x0a == sqr */
    {root_width,         root_height,         root_bits        }, /* \x0b == root */
    {pow10_width,        pow10_height,        pow10_bits       }, /* \x0c == pow10 */
    {exp_width,          exp_height,          exp_bits         }, /* \x0d == exp */
    {prog_width,         prog_height,         prog_bits        }, /* \x0e == << >> */
    {string_width,       string_height,       string_bits      }, /* \x0f == " " */
    {nl_width,           nl_height,           nl_bits          }, /* \x10 == New Line # 16 */
    {pi_width,           pi_height,           pi_bits          }, /* \x11 == pi */
    {angle_width,        angle_height,        angle_bits       }, /* \x12 == angle */
    {sqr_48gx_width,     sqr_48gx_height,     sqr_48gx_bits    }, /* \x13 == sqr gx */
    {root_48gx_width,    root_48gx_height,    root_48gx_bits   }, /* \x14 == root gx */
    {pow10_48gx_width,   pow10_48gx_height,   pow10_48gx_bits  }, /* \x15 == pow10 gx */
    {exp_48gx_width,     exp_48gx_height,     exp_48gx_bits    }, /* \x16 == exp gx */
    {parens_48gx_width,  parens_48gx_height,  parens_48gx_bits }, /* \x17 == ( ) gx */
    {hash_48gx_width,    hash_48gx_height,    hash_48gx_bits   }, /* \x18 == # gx */
    {bracket_48gx_width, bracket_48gx_height, bracket_48gx_bits}, /* \x19 == [] gx */
    {under_48gx_width,   under_48gx_height,   under_48gx_bits  }, /* \x1a == _ gx */
    {prog_48gx_width,    prog_48gx_height,    prog_48gx_bits   }, /* \x1b == << >> gx */
    {quote_48gx_width,   quote_48gx_height,   quote_48gx_bits  }, /* \x1c == " " gx */
    {curly_48gx_width,   curly_48gx_height,   curly_48gx_bits  }, /* \x1d == {} gx */
    {colon_48gx_width,   colon_48gx_height,   colon_48gx_bits  }, /* \x1e == :: gx */
    {angle_48gx_width,   angle_48gx_height,   angle_48gx_bits  }, /* \x1f == angle gx */
    {blank_width,        blank_height,        blank_bits       }, /* # 32 */
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {hash_width,         hash_height,         hash_bits        },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {lbrace_width,       lbrace_height,       lbrace_bits      },
    {rbrace_width,       rbrace_height,       rbrace_bits      },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {comma_width,        comma_height,        comma_bits       },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {slash_width,        slash_height,        slash_bits       },
    {zero_width,         zero_height,         zero_bits        }, /* # 48 */
    {one_width,          one_height,          one_bits         },
    {two_width,          two_height,          two_bits         },
    {three_width,        three_height,        three_bits       },
    {four_width,         four_height,         four_bits        },
    {five_width,         five_height,         five_bits        },
    {six_width,          six_height,          six_bits         },
    {seven_width,        seven_height,        seven_bits       },
    {eight_width,        eight_height,        eight_bits       },
    {nine_width,         nine_height,         nine_bits        },
    {small_colon_width,  small_colon_height,  small_colon_bits },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {equal_width,        equal_height,        equal_bits       },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }, /* # 64 */
    {A_width,            A_height,            A_bits           },
    {B_width,            B_height,            B_bits           },
    {C_width,            C_height,            C_bits           },
    {D_width,            D_height,            D_bits           },
    {E_width,            E_height,            E_bits           },
    {F_width,            F_height,            F_bits           },
    {G_width,            G_height,            G_bits           },
    {H_width,            H_height,            H_bits           },
    {I_width,            I_height,            I_bits           },
    {J_width,            J_height,            J_bits           },
    {K_width,            K_height,            K_bits           },
    {L_width,            L_height,            L_bits           },
    {M_width,            M_height,            M_bits           },
    {N_width,            N_height,            N_bits           },
    {O_width,            O_height,            O_bits           },
    {P_width,            P_height,            P_bits           }, /* # 80 */
    {Q_width,            Q_height,            Q_bits           },
    {R_width,            R_height,            R_bits           },
    {S_width,            S_height,            S_bits           },
    {T_width,            T_height,            T_bits           },
    {U_width,            U_height,            U_bits           },
    {V_width,            V_height,            V_bits           },
    {W_width,            W_height,            W_bits           },
    {X_width,            X_height,            X_bits           },
    {Y_width,            Y_height,            Y_bits           },
    {Z_width,            Z_height,            Z_bits           },
    {lbracket_width,     lbracket_height,     lbracket_bits    },
    {0,                  0,                   0                },
    {rbracket_width,     rbracket_height,     rbracket_bits    },
    {0,                  0,                   0                },
    {under_width,        under_height,        under_bits       },
    {0,                  0,                   0                }, /* # 96 */
    {0,                  0,                   0                }, /* a */
    {0,                  0,                   0                }, /* b */
    {0,                  0,                   0                }, /* c */
    {d_width,            d_height,            d_bits           },
    {e_width,            e_height,            e_bits           },
    {0,                  0,                   0                }, /* f */
    {0,                  0,                   0                }, /* g */
    {0,                  0,                   0                }, /* h */
    {i_width,            i_height,            i_bits           },
    {0,                  0,                   0                }, /* j */
    {0,                  0,                   0                }, /* k */
    {0,                  0,                   0                }, /* l */
    {0,                  0,                   0                }, /* m */
    {0,                  0,                   0                }, /* n */
    {0,                  0,                   0                }, /* o */
    {p_width,            p_height,            p_bits           },
    {0,                  0,                   0                }, /* q */
    {r_width,            r_height,            r_bits           },
    {s_width,            s_height,            s_bits           },
    {t_width,            t_height,            t_bits           },
    {0,                  0,                   0                }, /* u */
    {v_width,            v_height,            v_bits           },
    {w_width,            w_height,            w_bits           },
    {0,                  0,                   0                }, /* x */
    {y_width,            y_height,            y_bits           },
    {0,                  0,                   0                }, /* z */
    {lcurly_width,       lcurly_height,       lcurly_bits      },
    {0,                  0,                   0                },
    {rcurly_width,       rcurly_height,       rcurly_bits      },
    {0,                  0,                   0                },
    {0,                  0,                   0                }, /* 127 */
};

letter_t big_font[ 128 ] = {
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }, /* # 16 */
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }, /* # 32 */
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {big_font_dot_width, big_font_dot_height, big_font_dot_bits}, /* # 46 */
    {0,                  0,                   0                },
    {big_font_0_width,   big_font_0_height,   big_font_0_bits  }, /* # 48 */
    {big_font_1_width,   big_font_1_height,   big_font_1_bits  },
    {big_font_2_width,   big_font_2_height,   big_font_2_bits  },
    {big_font_3_width,   big_font_3_height,   big_font_3_bits  },
    {big_font_4_width,   big_font_4_height,   big_font_4_bits  },
    {big_font_5_width,   big_font_5_height,   big_font_5_bits  },
    {big_font_6_width,   big_font_6_height,   big_font_6_bits  },
    {big_font_7_width,   big_font_7_height,   big_font_7_bits  },
    {big_font_8_width,   big_font_8_height,   big_font_8_bits  },
    {big_font_9_width,   big_font_9_height,   big_font_9_bits  },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }, /* # 64 */
    {big_font_A_width,   big_font_A_height,   big_font_A_bits  },
    {0,                  0,                   0                },
    {big_font_C_width,   big_font_C_height,   big_font_C_bits  },
    {big_font_D_width,   big_font_D_height,   big_font_D_bits  },
    {big_font_E_width,   big_font_E_height,   big_font_E_bits  },
    {big_font_F_width,   big_font_F_height,   big_font_F_bits  },
    {big_font_G_width,   big_font_G_height,   big_font_G_bits  },
    {big_font_H_width,   big_font_H_height,   big_font_H_bits  },
    {big_font_I_width,   big_font_I_height,   big_font_I_bits  },
    {big_font_J_width,   big_font_J_height,   big_font_J_bits  },
    {0,                  0,                   0                },
    {big_font_L_width,   big_font_L_height,   big_font_L_bits  },
    {big_font_M_width,   big_font_M_height,   big_font_M_bits  },
    {big_font_N_width,   big_font_N_height,   big_font_N_bits  },
    {big_font_O_width,   big_font_O_height,   big_font_O_bits  },
    {big_font_P_width,   big_font_P_height,   big_font_P_bits  }, /* # 80 */
    {big_font_Q_width,   big_font_Q_height,   big_font_Q_bits  },
    {big_font_R_width,   big_font_R_height,   big_font_R_bits  },
    {big_font_S_width,   big_font_S_height,   big_font_S_bits  },
    {big_font_T_width,   big_font_T_height,   big_font_T_bits  },
    {0,                  0,                   0                },
    {big_font_V_width,   big_font_V_height,   big_font_V_bits  },
    {big_font_W_width,   big_font_W_height,   big_font_W_bits  },
    {big_font_X_width,   big_font_X_height,   big_font_X_bits  },
    {big_font_Y_width,   big_font_Y_height,   big_font_Y_bits  },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }, /* # 96 */
    {0,                  0,                   0                }, /* a */
    {0,                  0,                   0                }, /* b */
    {0,                  0,                   0                }, /* c */
    {0,                  0,                   0                }, /* d */
    {0,                  0,                   0                }, /* e */
    {0,                  0,                   0                }, /* f */
    {0,                  0,                   0                }, /* g */
    {0,                  0,                   0                }, /* h */
    {0,                  0,                   0                }, /* i */
    {0,                  0,                   0                }, /* j */
    {0,                  0,                   0                }, /* k */
    {0,                  0,                   0                }, /* l */
    {0,                  0,                   0                }, /* m */
    {0,                  0,                   0                }, /* n */
    {0,                  0,                   0                }, /* o */
    {0,                  0,                   0                }, /* p */
    {0,                  0,                   0                }, /* q */
    {0,                  0,                   0                }, /* r */
    {0,                  0,                   0                }, /* s */
    {0,                  0,                   0                }, /* t */
    {0,                  0,                   0                }, /* u */
    {0,                  0,                   0                }, /* v */
    {0,                  0,                   0                }, /* w */
    {0,                  0,                   0                }, /* x */
    {0,                  0,                   0                }, /* y */
    {0,                  0,                   0                }, /* z */
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }
};
