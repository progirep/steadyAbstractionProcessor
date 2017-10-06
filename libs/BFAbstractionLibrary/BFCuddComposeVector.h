#ifndef __BF_CUDD_COMPOSE_VECTOR_H__
#define __BF_CUDD_COMPOSE_VECTOR_H__

#include <cassert>
#include <cuddInt.h>

/**
 * @brief This class is used to store the parameters to a BDDCompose operation.
 */
class BFBddComposeVector {
private:
    BFBddManager *mgr;
    unsigned int nofVars;
    DdNode **nodes;
public:
    inline BFBddComposeVector() : mgr(nullptr), nofVars(0), nodes(nullptr) {}
    inline BFBddComposeVector(BFBddManager &_mgr) : mgr(&_mgr) {
        nofVars = 0;
        nodes = nullptr;
    }

    ~BFBddComposeVector() {
        if (nodes!=NULL) {
            for (unsigned int i=0;i<nofVars;i++) Cudd_RecursiveDeref(mgr->getMgr(),nodes[i]);
            free(nodes);
        }
    }

    /**
     * @brief Sets an elements of the ComposeVector to a
     * @param var
     * @param replacement
     */
    inline void set(BFBdd var, BFBdd replacement) {
        DdNode *varNode = var.getCuddNode();

        // Check that the given variable is really one...
        assert(Cudd_Regular(varNode)==varNode);
        unsigned int varIndex = varNode->index;
        assert(Cudd_T(varNode)==Cudd_ReadOne(mgr->getMgr()));
        assert(Cudd_E(varNode)==Cudd_Not(Cudd_ReadOne(mgr->getMgr())));

        // Increase the size of the node array if needed.
        if (varIndex>=nofVars) {
            nodes = (DdNode**)realloc(nodes, sizeof(size_t)*(varIndex+1));
            // Fill added values
            for (unsigned int i=nofVars;i<varIndex;i++) {
                nodes[i] = Cudd_bddIthVar(mgr->getMgr(),i);
                Cudd_Ref(nodes[i]);
            }
            nodes[varIndex] = replacement.getCuddNode();
            Cudd_Ref(nodes[varIndex]);
            nofVars = varIndex+1;
        } else {
            // Just replace the vector directly.
            Cudd_RecursiveDeref(mgr->getMgr(),nodes[varIndex]);
            nodes[varIndex] = replacement.getCuddNode();
            Cudd_Ref(nodes[varIndex]);
        }
    }

    /**
     * @brief This function resynchronizes the length of the vector to
     *        the number
     */
    inline void ensureSufficientSize() {
        unsigned int varIndex = Cudd_ReadSize(mgr->getMgr());
        assert(nofVars <= varIndex);
        if (varIndex>nofVars) {
            nodes = (DdNode**)realloc(nodes, sizeof(size_t)*(varIndex));
            // Fill added values
            for (unsigned int i=nofVars;i<varIndex;i++) {
                nodes[i] = Cudd_bddIthVar(mgr->getMgr(),i);
                Cudd_Ref(nodes[i]);
            }
        }
        nofVars = varIndex;
    }

    friend class BFBdd;

};

#endif
