// dp랑 <name>: 이거 하나의 문자열로 합치기 토큰사이에 공백넣는거 잊지말고

// static void postprocess_dp(void) {
//    tree_t *dp, *character;
//    char *name;
//    int i, j, len;

//    // len 이라는 이름 쓰면 헷갈릴수잇으니까 쓰지말기

//    dp = pt->child[1];
//    len = 0;

//    for (i = 0; i < dp->clen; i++) {
//       character = dp->child[i];
//       if (character->clen == 1)
//          continue;
//       for (j = 0; j < character->clen; j++)
//          len += character->child[j]->dsiz - 1;
//       len++;  // \0
//       name = malloc(len);
//       name[0] = '\0';
//       for (j = 0; j < character->clen; j++) {
//          strcat(name, character->child[j]->data);
//          tree_prune(character->child[i]);
//       }
//       (void) tree_graft(character, name, len, "CONTENT");
//    }
// }
