namespace core {
class ItemManager {
   private:
    ItemManager() = default;

   public:
    /**
             * Copy constructors stay empty, because of the Singleton
             */
    ItemManager(ItemManager const &) = delete;

    void operator=(ItemManager const &) = delete;

    /**
             * @return instance of the Singleton of the Project Manager
             */
    static ItemManager &getInstance() {
        static ItemManager instance;
        return instance;
    }
};
}  // namespace core