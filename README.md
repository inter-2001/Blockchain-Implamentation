Part 1 Project Setup
The Project setup for me involves utilising NeoVim and a custom compilation UI CTUI.
FTXUI(Sonzogni, 2026) was used for the UI portion of the Project, This is a relatively simple TUI (Terminal User Interface) system that can be used to create universal UI systems as due to the ANSI terminals ubiquity, it should work on most systems. It is also, due to being terminal only, usable over SSH, making it incredibly useful for headless systems.
Alongside FTXUI I have utilised cthash(Dusíková, 2026) for hashing and Crypto++(“Crypto++ Library 8.9 | Free C++ Class Library of Cryptographic Schemes,” n.d.) for the elliptic curve digital signing.
Overall Class Diagram
Part 2
The Block Class
The block class deviates slightly from the C# style implementation in that it doesn’t use the constructor and copy constructor to generate itself, it instead uses ChainBlock and GenisisBlock to initiate itself. This allows the class to be included with more ease in other classes, letting it be instantiated in memory, without being fully initialised. It also allows for it to be copied with ease.
Hashing
The Function in Figure 3 is the equivalent of the CreateHash function, it takes a string and returns the hash of said string. To hash each block therefore, one simply combines its structure into a single string and passes it.
This is usually not a great idea due to the possibility of Collision:
 and  
I don’t see this as an issue for this project however due to the consistent lengths of the data involved in the hashing as well as the significant length and number of variables. As such I would consider it almost impossible for collisions to occur given these circumstances. 
The BlockChain Class
The constructor for the BlockChain Class creates a temporary block, initialises it as a Genesis block, then pushes it to the back of the Blocks vector, as shown in figure 6.
Printing a Block to the UI
Printing a block has two steps, one is the PrintBlock function in the Block class, as shown in Fig7. This is called by the PrintBlock button in the main file as shown in Fig8. Also shown here is use of the Get_Block function, part of the BlockChain class, it simply retrieves the block at the given index. Figure 9 shows the UI output of this. A single block printed into the Main Display, the controls can be seen in the chain section with the block box being the selector for the index and print index printing it to the display. Also included for this part of the project is the function in Fig10. This converts C++ strings to Hexadecimal representation
Part 3
The Wallet Class
The Wallet class shown in Fig11 has two main purposes, to store a public key and to check the balance of said key. The actual logic of these functions is split between the BlockChain class and DigitalSigning.hpp header file. This is then implemented into the UI as shown in Fig12. Alongside the standard implementation I have added a save function for each key pair for saving a generated wallet to a file.
The Transaction Class
The transaction class as shown in Fig13 similarly to block doesn’t implement the initialisation directly into the constructor, this allows later for easier initialisation into other data structures and aids in the reward system also. The initialisation function as shown in Fig14 takes the requisite inputs, generating the Hash and Signature as described in the specification.
Processing Transactions and Transaction Pools
As can be seen in fig15, the UI elements to create a transaction have been created and display a transaction inline with the specification provided. Above in Fig5 you can see the presence of the Transaction pool. This has been implemented as a weighted que, leveraging the overloading of the greater and less than operators of the Transaction class to implement a weighted rewards system where the higher the time spent waiting and fee, the closer to the top of the list a transaction should appear.
Part 4
Generating New Blocks
As I implemented the Transaction Pool as a weighted que the function to grab blocks, shown in fig17, is rather simple. It can also handle not having a full set of transactions to handle by simply leaving the remaining values uninitialised. The value TransactionsInBlock is a constant value defining how many transactions are to be entered into each block, this defaults to 5.
Proof of Work
Instead of implementing nonce directly into HASH_STRING I elected to implement it by converting the data to a string and passing it to the function added to the end of the DataToHash, As previously discussed, due to the nature of the data, this is incredibly unlikely to cause collision.  The difficulty threshold is set as a global variable and defaults to 2 for testing, though this would be increased for an actual implementation of this system. The output of this can be seen in Fig16.
Rewards And Fees
Transaction Rewards
The reward for each transaction is seen here, calculated as the total amount being transacted multiplied by the time difference between when it was created and now in seconds. This could result in massive inflation if transactions are left for too long, but I think it would simply encourage people to mine more at peak transaction times when combined with low difficulty thresholds. The overloaded <> operators allow the weighted que to operate given these reward criteria.
Empty Block Rewards
The function here shows how the system calculates rewards and difficulty for empty blocks. If a block is not full it will increase in difficulty threshold by 1 for each blank entry, This increase in difficulty makes it harder to pump out near empty blocks, limiting inflation. But also allowing for the creation of new currency when there are no new pending transactions. This is a twofold system designed to increase interest in downtimes by allowing for mining empty blocks for diminishing rewards (1/ total blocks) whilst also decreasing energy overheads by making peak time mining the most profitable option due to far higher rewards being given per transaction.
Part 5
Validating the BlockChain Structure and Blocks
Validation is performed via a tiered system, Each block is Validated individually using the Fig21 algorithm. This is then done in sequence by the Fig22 algorithm, validating the whole chain.
Validating Balance
The Fig 23 function shows the system for validation of balance on its highest level. The Check Numeric Balance grabs a wallets balance and checks it against the given transaction, only adding it to the transaction pool if there is enough in the wallet to cover the transaction. The functions in fig24 give a better idea of how this is achieved, every transaction involving the wallet being checked is retrieved and placed into a map, This means that possible duplicates are negated, and once this has been done the amounts are tallied and returned through the CheckNumericBalance function. This can be seen working as part of the UI in fig25.
Merkle Roots
The Merkle root function as seen in Fig26 is used in the Chain Block function as seen in Fig20 as well as the Validate function. This means that it can effectively prevent tampering as it forms a part of the overall hash for the block and thus is stored in the subsequent hash, ensuring that the transactions are tamper proof.
Validating Transactions
The verify function shown in Fig27 internally verifies each transaction, ensuring that they cannot be tampered with due to the inclusion of the signature in the overall chains subsequent hashes.
Part 6
Task 1
My solution to this task was to set up a second option in the mine function that branches off into a number of thready equivalent to the defined constant ThreadCount in Utils.hpp. To prevent repeated hashing of the same data, I utilize a handler function that then repeatedly calls a sub-function for the nonce by having the sub-function be passed the hasher by copy. The MINE_HASH function simply returns a boolean indicating wether the nonce passed passes the check string for the given hash. The nonce is assigned based on each threads passed value, each iterates nonce ThreadCount values per run, giving each thread a multiple to search in. Once the MINE_HASH function returns true the handler returns the nonce value from its thread, which triggers the revised mine function to grab the value, ascertain the hash and return it, completing the hash function.  
This theoretically should provide a speedup of Original/ThreadCount, assuming each thread is running at the same speed as the original. This can be seen in the code in figures 28 and 29. Fig30 shows the flowchart for this function and fig.31 the testing output. Testing occurred across 4 cores as that is the number of physical cores on my system and thus offers the best speed up to test with. And shows a consistent 4x speedup.
Task 2
As can be seen in fig20, and has already been detailed in the Empty Block Rewards section, I have implemented a variable difficulty system designed to increase difficulty during times of low demand but encourage fast processing of transactions during peak times. When combined with the implemented fee system, reminiscent of the suggestions made in regard to etherium by (Roughgarden, 2020). This does also of course require an agreement as to the current time by all those in the chain, This already exists in-effect due to the block timestamps, and would require minimal extra checks to ensure consistency. The nature of the race to generate the next block removing any incentive for fudging timestamps.
Task3
In most block chains the Greedy approach to mining is by far the most sensible option for most miners, those with an interest in expanding the use of the blockchain might use altruistic and those who want to process there own interest might use address preference.
In the design of the system one should try to make an altruistic, or longest wait first, approach the most effective and that is what I propose to achieve with the altered rewards technique.
So to conclude, I think with many current chains, the greedy approach is incentivized, I think the best option for general design is to try to work with peoples greed and make the most profitable option also the best for the system. Proof of implementation is present in Fig32.
Task4
I had intended to run this system across a network but due to time constraints I shan’t have time. I would suggest therefore that the C++ implementation utilizing a non-standard UI system and implementing everything effectively from scratch, excluding the signing, hashing and UI core, could constitute the requirements for task 4, with the evidence being distributed through the rest of the document.

Bibliography
Crypto++ Library 8.9 | Free C++ Class Library of Cryptographic Schemes [WWW Document], n.d. URL https://www.cryptopp.com/ (accessed 5.15.26).
Dusíková, H., 2026. hanickadot/cthash.
Roughgarden, T., 2020. Transaction Fee Mechanism Design for the Ethereum Blockchain: An Economic Analysis of EIP-1559. https://doi.org/10.48550/arXiv.2012.00854
Sonzogni, A., 2026. ArthurSonzogni/FTXUI.
