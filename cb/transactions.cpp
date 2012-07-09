
// Dump all transactions affecting a specific address

#include <util.h>
#include <common.h>
#include <rmd160.h>
#include <string.h>
#include <callback.h>

struct Transactions:public Callback
{
    uint64_t sum;
    uint64_t nbTX;

    virtual bool needTXHash()
    {
        return true;
    }

    virtual int init(
        int argc,
        char *argv[]
    )
    {
        sum = 0;

        bool ok = (0==argc || 1==argc);
        if(!ok) return -1;

        loadKeyHash((const uint8_t*)argv[0]);
        return 0;
    }

    void move(
        const uint8_t *script,
        uint64_t      scriptSize,
        const uint8_t *txHash,
        uint64_t       value,
        bool           add,
        const uint8_t *downTXHash = 0
    )
    {
        uint8_t addrType[3];
        uint8_t pubKeyHash[kRIPEMD160ByteSize];
        int type = solveOutputScript(pubKeyHash, script, scriptSize, addrType);
        if(unlikely(type<0))
            return;

        const uint8_t *targetKeyHash = loadKeyHash();
        bool match = (0==memcmp(targetKeyHash, pubKeyHash, sizeof(pubKeyHash)));
        if(unlikely(match)) {

            printf("    ");
            showHex(downTXHash ? downTXHash : txHash);

            int64_t newSum = sum + value*(add ? 1 : -1);
            printf(
                " %24.08f %c %24.08f = %24.08f\n",
                sum*1e-8,
                add ? '+' : '-',
                value*1e-8,
                newSum*1e-8
            );
            sum = newSum;
            ++nbTX;
        }
    }

    virtual void endOutput(
        const uint8_t *p,
        uint64_t      value,
        const uint8_t *txHash,
        uint64_t      outputIndex,
        const uint8_t *outputScript,
        uint64_t      outputScriptSize
    )
    {
        move(
            outputScript,
            outputScriptSize,
            txHash,
            value,
            true
        );
    }

    virtual void edge(
        uint64_t      value,
        const uint8_t *upTXHash,
        uint64_t      outputIndex,
        const uint8_t *outputScript,
        uint64_t      outputScriptSize,
        const uint8_t *downTXHash,
        uint64_t      inputIndex,
        const uint8_t *inputScript,
        uint64_t      inputScriptSize
    )
    {
        move(
            outputScript,
            outputScriptSize,
            upTXHash,
            value,
            false,
            downTXHash
        );
    }

    virtual void startMap(
        const uint8_t *p
    )
    {

        printf("Dumping all transactions for address ");
        showHex(loadKeyHash(), kRIPEMD160ByteSize, false);

        uint8_t b58[128];
        hash160ToAddr(b58, loadKeyHash());
        printf(" %s\n\n", b58);

        printf("    Transaction                                                                    OldBalance                     Amount                 NewBalance\n");
        printf("    ===============================================================================================================================================\n");
    }

    virtual void endMap(
        const uint8_t *p
    )
    {
        printf(
            "    ===============================================================================================================================================\n"
            "\n"
            "    %" PRIu64 " transactions, final balance = %.08f\n"
            "\n",
            nbTX,
            sum*1e-8
        );
    }

    virtual const char *name()
    {
        return "transactions";
    }
};

static Transactions transactions;

