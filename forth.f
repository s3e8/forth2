: here      dp @ ;
: >mark     here 0 , ;
: >resolve  here swap ! ;

: if        ' 0branch , >mark  ; immediate
: then      dup here swap - swap ! ; immediate
: else      ' branch , >mark swap dup here swap - swap ! ; immediate