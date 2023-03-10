from ._base_optimizer import _BaseOptimizer
class SGD(_BaseOptimizer):
    def __init__(self, model, learning_rate=1e-4, reg=1e-3, momentum=0.9):
        super().__init__(model, learning_rate, reg)
        self.momentum = momentum

        # initialize the velocity terms for each weight
        self.vw = []
        self.vb = []
        for i in range(len(model.modules)):
            self.vw.append(0)
            self.vb.append(0)

    def update(self, model):
        '''
        Update model weights based on gradients
        :param model: The model to be updated
        :return: None, but the model weights should be updated
        '''
        #self.apply_regularization(model)

        for idx, m in enumerate(model.modules):
            if hasattr(m, 'weight'):
                #############################################################################
                # TODO:                                                                     #
                #    1) Momentum updates for weights                                        #
                #############################################################################
                self.vw[idx] = self.momentum * self.vw[idx] - self.learning_rate * m.dw
                m.weight += self.vw[idx]
                #############################################################################
                #                              END OF YOUR CODE                             #
                #############################################################################
            if hasattr(m, 'bias'):
                #############################################################################
                # TODO:                                                                     #
                #    1) Momentum updates for bias                                           #
                #############################################################################
                self.vb[idx] = self.momentum * self.vb[idx] - self.learning_rate * m.db
                m.bias += self.vb[idx]
                
                #############################################################################
                #                              END OF YOUR CODE                             #
                #############################################################################
